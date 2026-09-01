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
	virtual ~C7ZipArchiveOpenCallback() {}
	Z7_COM_UNKNOWN_IMP_3(
					IArchiveOpenVolumeCallback,
					ICryptoGetTextPassword,
					IArchiveOpenSetSubArchiveName
					);

	// IArchiveOpenCallback
	STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes) throw();
	STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes) throw();

	// IArchiveOpenVolumeCallback
	STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT *value) throw();
	STDMETHOD(GetStream)(const wchar_t *name, IInStream **inStream) throw();

	// ICryptoGetTextPassword
	STDMETHOD(CryptoGetTextPassword)(BSTR *password) throw();

	STDMETHOD(SetSubArchiveName(const wchar_t *name)) throw()		{
		_subArchiveMode = true;
		_subArchiveName = name;
		TotalSize = 0;
		return  S_OK;
	}

public:
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
