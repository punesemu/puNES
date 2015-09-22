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
using namespace NWindows;

#include "stdlib.h"

#include "HelperFuncs.h"
#include "7ZipFunctions.h"
#include "7ZipDllHandler.h"
#include "7ZipCodecInfo.h"
#include "7ZipCompressCodecsInfo.h"

/*----------------------- C7ZipCompressCodecsInfo -------------------------*/
C7ZipCompressCodecsInfo::C7ZipCompressCodecsInfo(C7ZipLibrary * pLibrary) :
m_pLibrary(pLibrary),
m_CodecInfoArray(false)
{
    InitData();
}

C7ZipCompressCodecsInfo::~C7ZipCompressCodecsInfo()
{
}

void C7ZipCompressCodecsInfo::InitData()
{
    if (!m_pLibrary->IsInitialized())
        return;

    const C7ZipObjectPtrArray & handlers = 
        m_pLibrary->GetInternalObjectsArray();

    for(C7ZipObjectPtrArray::const_iterator it = handlers.begin(); 
        it != handlers.end(); it++)
    {
        C7ZipDllHandler * pHandler = dynamic_cast<C7ZipDllHandler *>(*it);

        if (pHandler != NULL)
        {
            const C7ZipObjectPtrArray & codecs = pHandler->GetCodecInfoArray();

            for(C7ZipObjectPtrArray::const_iterator itCodec = codecs.begin(); 
                itCodec != codecs.end(); itCodec++)
            {
                m_CodecInfoArray.push_back(*itCodec);
            }
        }
    }
}

HRESULT C7ZipCompressCodecsInfo::GetNumberOfMethods(UInt32 *numMethods)
{
    *numMethods = (UInt32)m_CodecInfoArray.size();

    return S_OK;
}

HRESULT C7ZipCompressCodecsInfo::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
    C7ZipCodecInfo * pCodec = dynamic_cast<C7ZipCodecInfo *>(m_CodecInfoArray[index]);

    if (propID == NMethodPropID::kDecoderIsAssigned)
    {
        NWindows::NCOM::CPropVariant propVariant;
        propVariant = pCodec->DecoderAssigned;
        propVariant.Detach(value);
        return S_OK;
    }
    if (propID == NMethodPropID::kEncoderIsAssigned)
    {
        NWindows::NCOM::CPropVariant propVariant;
        propVariant = pCodec->EncoderAssigned;
        propVariant.Detach(value);
        return S_OK;
    }
    return pCodec->Functions->v.GetMethodProperty(pCodec->CodecIndex, propID, value);
}

HRESULT C7ZipCompressCodecsInfo::CreateDecoder(UInt32 index, const GUID *interfaceID, void **coder)
{
    C7ZipCodecInfo * pCodec = dynamic_cast<C7ZipCodecInfo *>(m_CodecInfoArray[index]);

    if (pCodec->DecoderAssigned)
        return pCodec->Functions->v.CreateObject(&pCodec->Decoder,
            interfaceID,
            coder);
    return S_OK;
}

HRESULT C7ZipCompressCodecsInfo::CreateEncoder(UInt32 index, const GUID *interfaceID, void **coder)
{
    C7ZipCodecInfo * pCodec = dynamic_cast<C7ZipCodecInfo *>(m_CodecInfoArray[index]);

    if (pCodec->EncoderAssigned)
        return pCodec->Functions->v.CreateObject(&pCodec->Encoder,
            interfaceID,
            coder);
    return S_OK;
}
