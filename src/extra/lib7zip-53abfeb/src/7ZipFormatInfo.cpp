#if !defined(_WIN32) && !defined(_OS2)
#include "CPP/myWindows/StdAfx.h"
#include "CPP/include_windows/windows.h"
#endif

#include "C/7zVersion.h"
#include "CPP/Common/Common.h"
#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"

#if MY_VER_MAJOR >= 15
#include "CPP/Common/MyBuffer.h"
#else
#include "CPP/Common/Buffer.h"
#endif

#if MY_VER_MAJOR >= 15
#define NArchiveEnumPrefix NArchive::NHandlerPropID
#else
#define NArchiveEnumPrefix NArchive
#endif

using namespace NWindows;

#include "lib7zip.h"

#include "HelperFuncs.h"
#include "7ZipFunctions.h"
#include "7ZipFormatInfo.h"

#if MY_VER_MAJOR >= 15
static bool ParseSignatures(const Byte *data, unsigned size, CObjectVector<CByteBuffer> &signatures)
{
  signatures.Clear();
  while (size > 0)
  {
    unsigned len = *data++;
    size--;
    if (len > size)
      return false;
    signatures.AddNew().CopyFrom(data, len);
    data += len;
    size -= len;
  }
  return true;
}
#endif

/*------------------------ C7ZipFormatInfo ---------------------*/
C7ZipFormatInfo::C7ZipFormatInfo()
    : m_StartSignature()
    , m_FinishSignature()
{
    m_Name.clear();
    memset(&m_ClassID,0,sizeof(GUID));
    m_UpdateEnabled = false;
    m_KeepName = false;

#if MY_VER_MAJOR < 15
    m_StartSignature.SetCapacity(0);
	m_FinishSignature.SetCapacity(0);
#endif

    Exts.clear();
    AddExts.clear();
}

C7ZipFormatInfo::~C7ZipFormatInfo()
{
}

bool LoadFormats(pU7ZipFunctions pFunctions, C7ZipObjectPtrArray & formats)
{
    if (pFunctions->v.GetHandlerProperty == NULL &&
        pFunctions->v.GetHandlerProperty2 == NULL)
    {
        return false;
    }

    UInt32 numFormats = 1;

    if (pFunctions->v.GetNumberOfFormats != NULL)
    {
        RBOOLOK(pFunctions->v.GetNumberOfFormats(&numFormats));
    }

    if (pFunctions->v.GetHandlerProperty2 == NULL)
        numFormats = 1;

    for(UInt32 i = 0; i < numFormats; i++)
    {
        wstring name;
        bool updateEnabled = false;
        bool keepName = false;
        GUID classID;
        wstring ext, addExt;

        if (ReadStringProp(pFunctions->v.GetHandlerProperty,
            pFunctions->v.GetHandlerProperty2, i, NArchiveEnumPrefix::kName, name) != S_OK)
            continue;

        NWindows::NCOM::CPropVariant prop;
        if (ReadProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2,
            i, NArchiveEnumPrefix::kClassID, prop) != S_OK)
            continue;
        if (prop.vt != VT_BSTR)
            continue;

        classID = *(const GUID *)prop.bstrVal;

        if (ReadStringProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2,
            i, NArchiveEnumPrefix::kExtension, ext) != S_OK)
            continue;

        if (ReadStringProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2,
            i, NArchiveEnumPrefix::kAddExtension, addExt) != S_OK)
            continue;

        ReadBoolProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2, i,
            NArchiveEnumPrefix::kUpdate, updateEnabled);

        if (updateEnabled)
        {
            ReadBoolProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2,
                i, NArchiveEnumPrefix::kKeepName, keepName);
        }

        C7ZipFormatInfo * pInfo = new C7ZipFormatInfo();

#if MY_VER_MAJOR >= 15
        if (ReadProp(pFunctions->v.GetHandlerProperty,
                     pFunctions->v.GetHandlerProperty2, i, NArchiveEnumPrefix::kSignature, prop) == S_OK) {
#else
        if (ReadProp(pFunctions->v.GetHandlerProperty,
                     pFunctions->v.GetHandlerProperty2, i, NArchiveEnumPrefix::kStartSignature, prop) == S_OK) {
#endif
          if (prop.vt == VT_BSTR) {
            UINT len = ::SysStringByteLen(prop.bstrVal);
#if MY_VER_MAJOR >= 15
            pInfo->m_StartSignature.CopyFrom((const Byte *)prop.bstrVal, len);
#else
            pInfo->m_StartSignature.SetCapacity(len);
            memmove(pInfo->m_StartSignature, prop.bstrVal, len);
#endif
#if MY_VER_MAJOR >= 15
            if (len > 0)
                pInfo->Signatures.Add(pInfo->m_StartSignature);
#endif
          }
        }

#if MY_VER_MAJOR >= 15
        if (ReadProp(pFunctions->v.GetHandlerProperty,
                     pFunctions->v.GetHandlerProperty2, i, NArchiveEnumPrefix::kMultiSignature, prop) == S_OK) {
#else
        if (ReadProp(pFunctions->v.GetHandlerProperty,
                     pFunctions->v.GetHandlerProperty2, i, NArchiveEnumPrefix::kFinishSignature, prop) == S_OK) {
#endif
          if (prop.vt == VT_BSTR) {
            UINT len = ::SysStringByteLen(prop.bstrVal);
#if MY_VER_MAJOR >= 15
            pInfo->m_FinishSignature.CopyFrom((const Byte *)prop.bstrVal, len);
            ParseSignatures(pInfo->m_FinishSignature,
                            (unsigned)pInfo->m_FinishSignature.Size(),
                            pInfo->Signatures);
#else
            pInfo->m_FinishSignature.SetCapacity(len);
            memmove(pInfo->m_FinishSignature, prop.bstrVal, len);
#endif
          }
        }

#if MY_VER_MAJOR >= 15
        if (ReadProp(pFunctions->v.GetHandlerProperty,
                     pFunctions->v.GetHandlerProperty2, i,
                     NArchiveEnumPrefix::kSignatureOffset, prop) == S_OK) {
            if (prop.vt == VT_UI4) {
                pInfo->SignatureOffset = prop.ulVal;
            }
            else {
                pInfo->SignatureOffset = 0;
            }
        }
        else {
            pInfo->SignatureOffset = 0;
        }
#endif
        pInfo->m_Name = name;
        pInfo->m_KeepName = keepName;
        pInfo->m_ClassID = classID;
        pInfo->m_UpdateEnabled = updateEnabled;

        SplitString(ext, pInfo->Exts);
        SplitString(addExt, pInfo->AddExts);

        pInfo->FormatIndex = i;
        formats.push_back(pInfo);
    }

    return true;
}
