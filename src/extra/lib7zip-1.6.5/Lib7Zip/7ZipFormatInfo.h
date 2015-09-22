#ifndef __7ZIP_FORMAT_INFO_H__
#define __7ZIP_FORMAT_INFO_H__

class C7ZipFormatInfo : public virtual C7ZipObject
{
public:
  C7ZipFormatInfo();
  virtual ~C7ZipFormatInfo();

public:
  wstring m_Name;
  GUID m_ClassID;
  bool m_UpdateEnabled;
  bool m_KeepName;

  WStringArray Exts;
  WStringArray AddExts;

  CByteBuffer m_StartSignature;
  CByteBuffer m_FinishSignature;

  int FormatIndex;
};

#endif //__7ZIP_FORMAT_INFO_H__
