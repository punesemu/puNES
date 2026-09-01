#ifndef __7ZIP_DLL_HANDLER_H__
#define __7ZIP_DLL_HANDLER_H__

class C7ZipDllHandler : 
    public virtual C7ZipObject
{
public:
    C7ZipDllHandler(C7ZipLibrary * pLibrary, void * pHandler);
    virtual ~C7ZipDllHandler();

public:
    bool GetSupportedExts(WStringArray & exts);
    bool OpenArchive(C7ZipInStream * pInStream, 
					 C7ZipMultiVolumes * pMultiVolumes, 
					 C7ZipArchive ** ppArchive, 
					 const wstring & passwd,
					 HRESULT * pResult,
                     bool fCheckFileTypeBySignature);
    bool IsInitialized() const { return m_bInitialized; }
    C7ZipLibrary * GetLibrary() const { return m_pLibrary; }
    const C7ZipObjectPtrArray & GetFormatInfoArray() const { return m_FormatInfoArray; }
    const C7ZipObjectPtrArray & GetCodecInfoArray() const { return m_CodecInfoArray; }
    pU7ZipFunctions GetFunctions() const { return const_cast<pU7ZipFunctions>(&m_Functions); }

    wstring GetHandlerPath() const;

private:
    C7ZipLibrary * m_pLibrary;
    bool m_bInitialized;
    void * m_pHandler;
    U7ZipFunctions m_Functions;
    C7ZipObjectPtrArray m_CodecInfoArray;
    C7ZipObjectPtrArray m_FormatInfoArray;

    void Initialize();
    void Deinitialize();
};

#endif //__7ZIP_DLL_HANDLER_H__

