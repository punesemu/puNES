// Windows/FileIO.h

#ifndef __WINDOWS_FILEIO_H
#define __WINDOWS_FILEIO_H

#include <Common/MyString.h>

#ifndef _WIN32

#define FILE_SHARE_READ	1
#define FILE_SHARE_WRITE 2

#define FILE_BEGIN	SEEK_SET
#define FILE_CURRENT	SEEK_CUR
#define FILE_END	SEEK_END
#define INVALID_SET_FILE_POINTER	((DWORD)-1)

#define CREATE_NEW	  1
#define CREATE_ALWAYS	  2
#define OPEN_EXISTING	  3
#define OPEN_ALWAYS	  4
/* #define TRUNCATE_EXISTING 5 */

#endif

namespace NWindows {
namespace NFile {
namespace NIO {


class CFileBase
{
protected:
  int     _fd;
  AString _unix_filename;
  time_t   _lastAccessTime;
  time_t   _lastWriteTime;
#ifdef ENV_HAVE_LSTAT
  int     _size;
  char    _buffer[MAX_PATHNAME_LEN+1];
  int     _offset;
#endif

  bool Create(LPCSTR fileName, DWORD desiredAccess,
      DWORD shareMode, DWORD creationDisposition,  DWORD flagsAndAttributes,bool ignoreSymbolicLink=false);
  bool Create(LPCWSTR fileName, DWORD desiredAccess,
      DWORD shareMode, DWORD creationDisposition,  DWORD flagsAndAttributes,bool ignoreSymbolicLink=false);

public:
  CFileBase(): _fd(-1) {};
  virtual ~CFileBase();

  virtual bool Close();

  bool GetLength(UINT64 &length) const;

  bool Seek(INT64 distanceToMove, DWORD moveMethod, UINT64 &newPosition);
  bool Seek(UINT64 position, UINT64 &newPosition);
};

class CInFile: public CFileBase
{
public:
  bool Open(LPCTSTR fileName, DWORD shareMode, DWORD creationDisposition,  DWORD flagsAndAttributes);
  bool OpenShared(LPCTSTR fileName, bool /* shareForWrite */ ,bool ignoreSymbolicLink=false) {
    return Open(fileName,ignoreSymbolicLink);
  }
  bool Open(LPCTSTR fileName,bool ignoreSymbolicLink=false);
  #ifndef _UNICODE
  bool Open(LPCWSTR fileName, DWORD shareMode, DWORD creationDisposition,  DWORD flagsAndAttributes);
  bool OpenShared(LPCWSTR fileName, bool /* shareForWrite */ ,bool ignoreSymbolicLink=false) {
    return Open(fileName,ignoreSymbolicLink);
  }
  bool Open(LPCWSTR fileName,bool ignoreSymbolicLink=false);
  #endif
  bool ReadPart(void *data, UINT32 size, UINT32 &processedSize);
  bool Read(void *data, UINT32 size, UINT32 &processedSize);
};

class COutFile: public CFileBase
{
public:
  bool Open(LPCTSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool Open(LPCTSTR fileName, DWORD creationDisposition);
  bool Create(LPCTSTR fileName, bool createAlways);

  #ifndef _UNICODE
  bool Open(LPCWSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool Open(LPCWSTR fileName, DWORD creationDisposition);
  bool Create(LPCWSTR fileName, bool createAlways);
  #endif

  /*
  void SetOpenCreationDisposition(DWORD creationDisposition)
    { m_CreationDisposition = creationDisposition; }
  void SetOpenCreationDispositionCreateAlways()
    { m_CreationDisposition = CREATE_ALWAYS; }
  */

  bool SetTime(const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime);
  bool SetMTime(const FILETIME *mTime);
  bool WritePart(const void *data, UINT32 size, UINT32 &processedSize);
  bool Write(const void *data, UINT32 size, UINT32 &processedSize);
  bool SetEndOfFile();
  bool SetLength(UINT64 length);
};

}}}

#endif
