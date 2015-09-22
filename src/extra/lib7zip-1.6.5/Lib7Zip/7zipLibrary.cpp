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

#ifndef _WIN32_WCE
#include <errno.h>
#else
#include <winerror.h>
#endif

#if !defined(_WIN32)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "dlfcn.h"
#include "unistd.h"
#endif

#include "HelperFuncs.h"
#include "7ZipFunctions.h"
#include "7ZipDllHandler.h"
#include "OSFunctions.h"
#include "7ZipArchiveOpenCallback.h"

/*-------------- const defines ---------------------------*/
const wchar_t kAnyStringWildcard = '*';

/*-------------- static functions ------------------------*/
extern bool LoadDllFromFolder(C7ZipDllHandler * pMainHandler, const wstring & folder_name, C7ZipObjectPtrArray & handlers);
static lib7zip::ErrorCodeEnum HResultToErrorCode(HRESULT hr);

/*------------------------ C7ZipLibrary ---------------------*/

C7ZipLibrary::C7ZipLibrary() :
m_bInitialized(false)
{
}

C7ZipLibrary::~C7ZipLibrary()
{
    Deinitialize();
}

void C7ZipLibrary::Deinitialize()
{
    if (!m_bInitialized)
        return;

    m_InternalObjectsArray.clear();

    m_bInitialized = false;
}

bool C7ZipLibrary::Initialize()
{
    if (m_bInitialized)
        return true;

    void * pHandler = Load7ZLibrary(L"7z");

    if (pHandler == NULL)
        return false;

    C7ZipDllHandler * p7ZipHandler = new C7ZipDllHandler(this, pHandler);

    if (p7ZipHandler->IsInitialized())
    {
        m_InternalObjectsArray.push_back(p7ZipHandler);

        m_bInitialized = true;

        LoadDllFromFolder(p7ZipHandler, L"Codecs", m_InternalObjectsArray);
        LoadDllFromFolder(p7ZipHandler, L"Formats", m_InternalObjectsArray);
    }
    else
    {
        delete p7ZipHandler;
        m_bInitialized = false;
    }

    return m_bInitialized;
}

bool C7ZipLibrary::GetSupportedExts(WStringArray & exts)
{
    exts.clear();

    if (!m_bInitialized)
        return false;

    for(C7ZipObjectPtrArray::iterator it = m_InternalObjectsArray.begin(); 
        it != m_InternalObjectsArray.end(); it++)
    {
        C7ZipDllHandler * pHandler = dynamic_cast<C7ZipDllHandler *>(*it);

        if (pHandler != NULL)
        {
            pHandler->GetSupportedExts(exts);
        }
    }

    return true;
}

bool C7ZipLibrary::OpenArchive(C7ZipInStream * pInStream, C7ZipArchive ** ppArchive, 
							   const wstring & passwd, bool fCheckFileTypeBySignature)
{
    if (!m_bInitialized) {
		m_LastError = lib7zip::LIB7ZIP_NOT_INITIALIZE;
        return false;
	}

    for(C7ZipObjectPtrArray::iterator it = m_InternalObjectsArray.begin(); 
        it != m_InternalObjectsArray.end(); it++)
    {
        C7ZipDllHandler * pHandler = dynamic_cast<C7ZipDllHandler *>(*it);
		HRESULT hr = S_OK;

        if (pHandler != NULL && pHandler->OpenArchive(pInStream, NULL, ppArchive, 
                                                      passwd, &hr,
                                                      fCheckFileTypeBySignature))
        {
			if (*ppArchive)
				 (*ppArchive)->SetArchivePassword(passwd);
			m_LastError = HResultToErrorCode(hr);
            return true;
        }

		m_LastError = HResultToErrorCode(hr);

		if (m_LastError == lib7zip::LIB7ZIP_NEED_PASSWORD)
			return false;
    }

	m_LastError = lib7zip::LIB7ZIP_NOT_SUPPORTED_ARCHIVE;
    return false;
}

bool C7ZipLibrary::OpenArchive(C7ZipInStream * pInStream, C7ZipArchive ** ppArchive, bool fCheckFileTypeBySignature)
{
  return OpenArchive(pInStream, ppArchive, L"", fCheckFileTypeBySignature);
}

bool C7ZipLibrary::OpenMultiVolumeArchive(C7ZipMultiVolumes * pMultiVolumes, C7ZipArchive ** ppArchive,
										  const wstring & passwd, bool fCheckFileTypeBySignature)
{
    if (!m_bInitialized) {
		m_LastError = lib7zip::LIB7ZIP_NOT_INITIALIZE;
        return false;
	}

    for(C7ZipObjectPtrArray::iterator it = m_InternalObjectsArray.begin(); 
        it != m_InternalObjectsArray.end(); it++)
    {
        C7ZipDllHandler * pHandler = dynamic_cast<C7ZipDllHandler *>(*it);
		HRESULT hr = S_OK;

        if (pHandler != NULL && pHandler->OpenArchive(NULL, pMultiVolumes, ppArchive, 
                                                      passwd, &hr, 
                                                      fCheckFileTypeBySignature))
        {
			if (*ppArchive)
				(*ppArchive)->SetArchivePassword(passwd);
			m_LastError = HResultToErrorCode(hr);
            return true;
        }

		m_LastError = HResultToErrorCode(hr);

		if (m_LastError == lib7zip::LIB7ZIP_NEED_PASSWORD)
			return false;
    }

	m_LastError = lib7zip::LIB7ZIP_NOT_SUPPORTED_ARCHIVE;
    return false;
}

bool C7ZipLibrary::OpenMultiVolumeArchive(C7ZipMultiVolumes * pInStream, C7ZipArchive ** ppArchive, 
                                          bool fCheckFileTypeBySignature)
{
  return OpenMultiVolumeArchive(pInStream, ppArchive, L"", fCheckFileTypeBySignature);
}

static lib7zip::ErrorCodeEnum HResultToErrorCode(HRESULT hr)
{
	switch(hr){
	case S_OK:
		return lib7zip::LIB7ZIP_NO_ERROR;
	case E_NEEDPASSWORD:
		return lib7zip::LIB7ZIP_NEED_PASSWORD;
	case S_FALSE:
	case E_NOTIMPL:
	case E_NOINTERFACE:
	case E_ABORT:
	case E_FAIL:
	case STG_E_INVALIDFUNCTION:
	case E_OUTOFMEMORY:
	case E_INVALIDARG:
	default:
		return lib7zip::LIB7ZIP_UNKNOWN_ERROR;
	}
}
