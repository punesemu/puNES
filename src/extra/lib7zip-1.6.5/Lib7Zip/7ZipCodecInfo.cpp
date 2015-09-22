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

#include "7ZipCodecInfo.h"

/*------------------------ C7ZipCodecInfo ---------------------*/
C7ZipCodecInfo::C7ZipCodecInfo()
{
    m_Name.clear();
    memset(&m_ClassID,0,sizeof(GUID));

    memset(&Encoder,0, sizeof(GUID));
    EncoderAssigned = false;

    memset(&Decoder,0, sizeof(GUID));
    DecoderAssigned = false;
}

C7ZipCodecInfo::~C7ZipCodecInfo()
{
}

bool LoadCodecs(pU7ZipFunctions pFunctions, C7ZipObjectPtrArray & codecInfos)
{
    if (pFunctions->v.CreateObject == NULL)
        return false;

    if (pFunctions->v.GetMethodProperty == NULL)
        return false;

    UInt32 numMethods = 0;
    RBOOLOK(pFunctions->v.GetNumberOfMethods(&numMethods));

    for(UInt32 i = 0; i < numMethods; i++)
    {
        wstring name = L"";
        GUID classID;
        memset(&classID, 0, sizeof(GUID));

/*
        if(GetMethodPropertyString(pFunctions->v.GetMethodProperty, i, 
            NMethodPropID::kName, name) != S_OK)
            continue;

        if (GetMethodPropertyGUID(pFunctions->v.GetMethodProperty, i, 
            NMethodPropID::kID, classID) != S_OK)
            continue;
*/

        GUID encoder, decoder;
        bool encoderIsAssigned, decoderIsAssigned;

        if (GetCoderClass(pFunctions->v.GetMethodProperty, i, 
            NMethodPropID::kEncoder, encoder, encoderIsAssigned) != S_OK)
            continue;
        if (GetCoderClass(pFunctions->v.GetMethodProperty, i, 
            NMethodPropID::kDecoder, decoder, decoderIsAssigned) != S_OK)
            continue;

        C7ZipCodecInfo * pInfo = new C7ZipCodecInfo();
        pInfo->Functions = pFunctions;

        pInfo->m_Name = name;
        pInfo->m_ClassID = classID;

        pInfo->Encoder = encoder;
        pInfo->EncoderAssigned = encoderIsAssigned;

        pInfo->Decoder = decoder;
        pInfo->DecoderAssigned = decoderIsAssigned;

        pInfo->CodecIndex = i;
        codecInfos.push_back(pInfo);
    }

    return true;
}
