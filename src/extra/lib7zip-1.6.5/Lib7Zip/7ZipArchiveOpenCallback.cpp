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

#include "7ZipArchiveOpenCallback.h"
#include "7ZipInStreamWrapper.h"

/*--------------------C7ZipArchiveOpenCallback------------------*/
STDMETHODIMP C7ZipArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
    return S_OK;
}

STDMETHODIMP C7ZipArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
    return S_OK;
}

STDMETHODIMP C7ZipArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
    if (!PasswordIsDefined) {
        return E_NEEDPASSWORD;
    }

#ifdef _WIN32
    return StringToBstr(Password.c_str(), password);
#else
	CMyComBSTR temp(Password.c_str());

	*password = temp.MyCopy();

	return S_OK;
#endif
}

STDMETHODIMP C7ZipArchiveOpenCallback::GetProperty(PROPID propID, PROPVARIANT *value) 
{
	COM_TRY_BEGIN
	NCOM::CPropVariant prop;
	if (_subArchiveMode)
		switch(propID)
			{
			case kpidName: prop = _subArchiveName.c_str(); break;
			}
	else
		switch(propID)
			{
			case kpidName:  
				{
					if (m_bMultiVolume) {
						prop = m_pMultiVolumes->GetFirstVolumeName().c_str(); 
					}
				}
				break;
			case kpidIsDir: prop = false; break;
			case kpidSize:
				{
					if (m_bMultiVolume) {
						prop = m_pMultiVolumes->GetCurrentVolumeSize();
					}
				}
				break;
			case kpidAttrib: prop = (UInt32)0; break;
			case kpidCTime: prop = 0; break;
			case kpidATime: prop = 0; break;
			case kpidMTime: prop = 0; break;
			}

	prop.Detach(value);
	return S_OK;
	COM_TRY_END
}

STDMETHODIMP C7ZipArchiveOpenCallback::GetStream(const wchar_t *name, IInStream **inStream)
{
	C7ZipInStream * pInStream = NULL;
	if (m_bMultiVolume) {
		if (!m_pMultiVolumes->MoveToVolume(name))
			return S_FALSE;

		pInStream = m_pMultiVolumes->OpenCurrentVolumeStream();
	} else {
		return S_FALSE;
	}

    C7ZipInStreamWrapper * pArchiveStream = new C7ZipInStreamWrapper(pInStream);

    CMyComPtr<IInStream> inStreamTemp(pArchiveStream); 
	*inStream = inStreamTemp.Detach();
	return S_OK;
}
