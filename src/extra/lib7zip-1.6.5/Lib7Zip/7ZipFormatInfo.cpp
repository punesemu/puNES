#include "lib7zip.h"

#ifdef S_OK
#undef S_OK
#endif

#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"
#include "CPP/Common/Buffer.h"

using namespace NWindows;

#include "HelperFuncs.h"
#include "7ZipFunctions.h"
#include "7ZipFormatInfo.h"

/*------------------------ C7ZipFormatInfo ---------------------*/
C7ZipFormatInfo::C7ZipFormatInfo()
{
    m_Name.clear();
    memset(&m_ClassID,0,sizeof(GUID));
    m_UpdateEnabled = false;
    m_KeepName = false;
    m_StartSignature.SetCapacity(0);
	m_FinishSignature.SetCapacity(0);

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
            pFunctions->v.GetHandlerProperty2, i, NArchive::kName, name) != S_OK)
            continue;

        NWindows::NCOM::CPropVariant prop;
        if (ReadProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2, 
            i, NArchive::kClassID, prop) != S_OK)
            continue;
        if (prop.vt != VT_BSTR)
            continue;

        classID = *(const GUID *)prop.bstrVal;

        if (ReadStringProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2, 
            i, NArchive::kExtension, ext) != S_OK)
            continue;

        if (ReadStringProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2, 
            i, NArchive::kAddExtension, addExt) != S_OK)
            continue;

        ReadBoolProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2, i, 
            NArchive::kUpdate, updateEnabled);

        if (updateEnabled)
        {
            ReadBoolProp(pFunctions->v.GetHandlerProperty, pFunctions->v.GetHandlerProperty2, 
                i, NArchive::kKeepName, keepName);
        }

        C7ZipFormatInfo * pInfo = new C7ZipFormatInfo();

        if (ReadProp(pFunctions->v.GetHandlerProperty, 
                     pFunctions->v.GetHandlerProperty2, i, NArchive::kStartSignature, prop) == S_OK) {
          if (prop.vt == VT_BSTR) {
            UINT len = ::SysStringByteLen(prop.bstrVal);
            pInfo->m_StartSignature.SetCapacity(len);
            memmove(pInfo->m_StartSignature, prop.bstrVal, len);
          }
        }

        if (ReadProp(pFunctions->v.GetHandlerProperty, 
                     pFunctions->v.GetHandlerProperty2, i, NArchive::kFinishSignature, prop) == S_OK) {
          if (prop.vt == VT_BSTR) {
            UINT len = ::SysStringByteLen(prop.bstrVal);
            pInfo->m_FinishSignature.SetCapacity(len);
            memmove(pInfo->m_FinishSignature, prop.bstrVal, len);
          }
        }
		
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

