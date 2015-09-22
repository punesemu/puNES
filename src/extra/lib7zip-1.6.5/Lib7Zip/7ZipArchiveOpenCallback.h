#ifndef __7ZIP_ARCHIVE_OPEN_CALLBACK_H__
#define  __7ZIP_ARCHIVE_OPEN_CALLBACK_H__

#define E_NEEDPASSWORD ((HRESULT)0x80040001L)

class C7ZipArchiveOpenCallback:
public IArchiveOpenCallback,
    public ICryptoGetTextPassword,
	public IArchiveOpenVolumeCallback,
	public IArchiveOpenSetSubArchiveName,
    public CMyUnknownImp
{
 public:
	MY_UNKNOWN_IMP3(
					IArchiveOpenVolumeCallback,
					ICryptoGetTextPassword,
					IArchiveOpenSetSubArchiveName
					);

	INTERFACE_IArchiveOpenCallback(;);
	INTERFACE_IArchiveOpenVolumeCallback(;);

	STDMETHOD(CryptoGetTextPassword)(BSTR *password);

	STDMETHOD(SetSubArchiveName(const wchar_t *name))		{
		_subArchiveMode = true;
		_subArchiveName = name;
		TotalSize = 0;
		return  S_OK;
	}

    bool PasswordIsDefined;
    wstring Password;

	wstring _subArchiveName;
	bool _subArchiveMode;
	UInt64 TotalSize;

    C7ZipMultiVolumes * m_pMultiVolumes;
	bool m_bMultiVolume;

 C7ZipArchiveOpenCallback(C7ZipMultiVolumes * pMultiVolumes) : PasswordIsDefined(false),
		_subArchiveMode(false), 
		m_pMultiVolumes(pMultiVolumes),
		m_bMultiVolume(pMultiVolumes != NULL) {
	}
};

#endif // __7ZIP_ARCHIVE_OPEN_CALLBACK_H__
