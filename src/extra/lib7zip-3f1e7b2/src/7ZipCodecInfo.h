#ifndef __7ZIP_CODEC_INFO_H__
#define __7ZIP_CODEC_INFO_H__

/*-------------- internal classes ------------------------*/
class C7ZipCodecInfo : public virtual C7ZipObject
{
public:
    C7ZipCodecInfo();
    virtual ~C7ZipCodecInfo();

public:
    wstring m_Name;
    GUID m_ClassID;

    GUID Encoder;
    bool EncoderAssigned;

    GUID Decoder;
    bool DecoderAssigned;

    int CodecIndex;

    pU7ZipFunctions Functions;
};

#endif //__7ZIP_CODEC_INFO_H__
