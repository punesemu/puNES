#ifndef __7ZIP_COMPRESS_CODECS_INFO_H__
#define __7ZIP_COMPRESS_CODECS_INFO_H__

class C7ZipCompressCodecsInfo : public ICompressCodecsInfo,
    public CMyUnknownImp,
    public virtual C7ZipObject
{
public:
    C7ZipCompressCodecsInfo(C7ZipLibrary * pLibrary);
    virtual ~C7ZipCompressCodecsInfo();

    MY_UNKNOWN_IMP1(ICompressCodecsInfo)

    STDMETHOD(GetNumberOfMethods)(UInt32 *numMethods);
    STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
    STDMETHOD(CreateDecoder)(UInt32 index, const GUID *interfaceID, void **coder);
    STDMETHOD(CreateEncoder)(UInt32 index, const GUID *interfaceID, void **coder);

    void InitData();
private:
    C7ZipLibrary * m_pLibrary;
    C7ZipObjectPtrArray m_CodecInfoArray;
};

#endif //__7ZIP_COMPRESS_CODECS_INFO_H__

