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
#include "7ZipDllHandler.h"
#include "7ZipCodecInfo.h"
#include "7ZipFormatInfo.h"

#ifdef _WIN32_WCE
#define myT(x) L##x
#else
#define myT(x) x
#endif

#include "OSFunctions.h"

extern bool LoadCodecs(pU7ZipFunctions pFunctions, C7ZipObjectPtrArray & codecInfos);
extern bool LoadFormats(pU7ZipFunctions pFunctions, C7ZipObjectPtrArray & formats);
extern HRESULT Lib7ZipOpenArchive(C7ZipLibrary * pLibrary,
								  C7ZipDllHandler * pHandler,
								  C7ZipInStream * pInStream,
								  C7ZipArchive ** ppArchive,
								  const wstring & passwd,
								  HRESULT * pResult,
                                  bool fCheckFileTypeBySignature);
extern HRESULT Lib7ZipOpenMultiVolumeArchive(C7ZipLibrary * pLibrary,
                                             C7ZipDllHandler * pHandler,
                                             C7ZipMultiVolumes * pMultiVolumes,
                                             C7ZipArchive ** ppArchive, 
                                             const wstring & passwd,	
                                             HRESULT * pResult,
                                             bool fCheckFileTypeBySignature);

/*------------------------------ C7ZipDllHandler ------------------------*/
C7ZipDllHandler::C7ZipDllHandler(C7ZipLibrary * pLibrary, void * pHandler) :
	m_pLibrary(pLibrary),
	m_pHandler(pHandler),
	m_bInitialized(false)
{
    Initialize();
}

C7ZipDllHandler::~C7ZipDllHandler()
{
    Deinitialize();
}

void C7ZipDllHandler::Initialize()
{
    pU7ZipFunctions pFunctions = &m_Functions;

    pFunctions->v.GetMethodProperty = 
        (GetMethodPropertyFunc)GetProcAddress((HMODULE)m_pHandler, myT("GetMethodProperty"));
    pFunctions->v.GetNumberOfMethods = 
        (GetNumberOfMethodsFunc)GetProcAddress((HMODULE)m_pHandler, myT("GetNumberOfMethods"));
    pFunctions->v.GetNumberOfFormats = 
        (GetNumberOfFormatsFunc)GetProcAddress((HMODULE)m_pHandler, myT("GetNumberOfFormats"));
    pFunctions->v.GetHandlerProperty = 
        (GetHandlerPropertyFunc)GetProcAddress((HMODULE)m_pHandler, myT("GetHandlerProperty"));
    pFunctions->v.GetHandlerProperty2 = 
        (GetHandlerPropertyFunc2)GetProcAddress((HMODULE)m_pHandler, myT("GetHandlerProperty2"));
    pFunctions->v.CreateObject = 
        (CreateObjectFunc)GetProcAddress((HMODULE)m_pHandler, myT("CreateObject"));
    pFunctions->v.SetLargePageMode = 
        (SetLargePageModeFunc)GetProcAddress((HMODULE)m_pHandler, myT("SetLargePageMode"));

    if (pFunctions->v.IsValid()) {
		m_bInitialized = LoadCodecs(pFunctions, m_CodecInfoArray);

		m_bInitialized |= LoadFormats(pFunctions, m_FormatInfoArray);
	}
}

void C7ZipDllHandler::Deinitialize()
{
    Free7ZLibrary((HMODULE)m_pHandler);

    m_CodecInfoArray.clear();
    m_FormatInfoArray.clear();

    m_bInitialized = false;
}

bool C7ZipDllHandler::GetSupportedExts(WStringArray & exts)
{
    if (!m_bInitialized)
        return false;

    for(C7ZipObjectPtrArray::iterator it = m_FormatInfoArray.begin(); it != m_FormatInfoArray.end(); it++) {
		C7ZipFormatInfo * pInfo = dynamic_cast<C7ZipFormatInfo *>(*it);

		for(WStringArray::iterator extIt = pInfo->Exts.begin(); extIt != pInfo->Exts.end(); extIt++) {
			exts.push_back(*extIt);
		}
	}

    return true;
}

bool C7ZipDllHandler::OpenArchive(C7ZipInStream * pInStream, C7ZipMultiVolumes * pMultiVolumes, 
								  C7ZipArchive ** ppArchive, const wstring & passwd,
								  HRESULT * pResult,
                                  bool fCheckFileTypeBySignature)
{
	if (pMultiVolumes != NULL)
		return Lib7ZipOpenMultiVolumeArchive(m_pLibrary, this, pMultiVolumes, ppArchive, 
											 passwd, pResult, fCheckFileTypeBySignature) == S_OK;
	else if (pInStream != NULL)
		return Lib7ZipOpenArchive(m_pLibrary, this, pInStream, ppArchive, 
								  passwd, pResult, fCheckFileTypeBySignature) == S_OK;
	
	return S_FALSE;
}

wstring C7ZipDllHandler::GetHandlerPath() const
{
#if defined(_WIN32)
	return ::GetHandlerPath((HMODULE)m_pHandler);
#else
	return ::GetHandlerPath((HMODULE)m_Functions.v.CreateObject);
#endif
}
