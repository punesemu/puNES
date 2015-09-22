// Windows/FileFind.h

#ifndef __WINDOWS_FILEFIND_H
#define __WINDOWS_FILEFIND_H

#include "../Common/MyString.h"
#include "FileName.h"
#include "Defs.h"

#include <sys/types.h> /* for DIR */
#include <dirent.h>

namespace NWindows {
namespace NFile {
namespace NFind {

namespace NAttributes
{
  inline bool IsReadOnly(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_READONLY) != 0; }
  inline bool IsHidden(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_HIDDEN) != 0; }
  inline bool IsSystem(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_SYSTEM) != 0; }
  inline bool IsDir(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0; }
  inline bool IsArchived(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_ARCHIVE) != 0; }
  inline bool IsCompressed(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_COMPRESSED) != 0; }
  inline bool IsEncrypted(DWORD attrib) { return (attrib & FILE_ATTRIBUTE_ENCRYPTED) != 0; }
}

class CFileInfoBase
{ 
  bool MatchesMask(UINT32 mask) const  { return ((Attrib & mask) != 0); }
public:
  UInt64 Size;
  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;
  DWORD Attrib;
  bool IsDevice;
  
  bool IsDir() const { return MatchesMask(FILE_ATTRIBUTE_DIRECTORY); }
};

class CFileInfo: public CFileInfoBase
{ 
public:
  AString Name; // FIXME CSysString Name;
  bool IsDots() const;
  bool Find(LPCSTR wildcard);
};

// FIXME #ifdef _UNICODE
// typedef CFileInfo CFileInfoW;
// #else
class CFileInfoW: public CFileInfoBase
{ 
public:
  UString Name;
  bool IsDots() const;
  bool Find(LPCWSTR wildcard);
};
// #endif

class CFindFile
{
  friend class CEnumerator;
  DIR *_dirp;
  AString _pattern;
  AString _directory;  
public:
  bool IsHandleAllocated() const { return (_dirp != 0); }
  CFindFile(): _dirp(0) {}
  ~CFindFile() {  Close(); }
  // bool FindFirst(LPCTSTR wildcard, CFileInfo &fileInfo);
  bool FindFirst(LPCSTR wildcard, CFileInfo &fileInfo);
  bool FindNext(CFileInfo &fileInfo);
  // FIXME #ifndef _UNICODE
  bool FindFirst(LPCWSTR wildcard, CFileInfoW &fileInfo);
  bool FindNext(CFileInfoW &fileInfo);
  // FIXME #endif
  bool Close();
};

bool FindFile(LPCSTR wildcard, CFileInfo &fileInfo);

bool DoesFileExist(LPCSTR name);
bool DoesDirExist(LPCTSTR name);
bool DoesFileOrDirExist(LPCSTR name);
// #ifndef _UNICODE
bool FindFile(LPCWSTR wildcard, CFileInfoW &fileInfo);
bool DoesFileExist(LPCWSTR name);
bool DoesDirExist(LPCWSTR name);
bool DoesFileOrDirExist(LPCWSTR name);
// #endif

class CEnumerator
{
  CFindFile _findFile;
  AString _wildcard; // FIXME CSysString _wildcard;
  bool NextAny(CFileInfo &fileInfo);
public:
  CEnumerator(): _wildcard(NName::kAnyStringWildcard) {}
  // FIXME CEnumerator(const CSysString &wildcard): _wildcard(wildcard) {}
  CEnumerator(const AString &wildcard): _wildcard(wildcard) {}
  bool Next(CFileInfo &fileInfo);
  bool Next(CFileInfo &fileInfo, bool &found);
};

// FIXME #ifdef _UNICODE
// typedef CEnumerator CEnumeratorW;
// #else
class CEnumeratorW
{
  CFindFile _findFile;
  UString _wildcard;
  bool NextAny(CFileInfoW &fileInfo);
public:
  CEnumeratorW(): _wildcard(NName::kAnyStringWildcard) {}
  CEnumeratorW(const UString &wildcard): _wildcard(wildcard) {}
  bool Next(CFileInfoW &fileInfo);
  bool Next(CFileInfoW &fileInfo, bool &found);
};
// FIXME #endif

}}}

#endif

