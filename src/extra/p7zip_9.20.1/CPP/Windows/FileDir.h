// Windows/FileDir.h

#ifndef __WINDOWS_FILEDIR_H
#define __WINDOWS_FILEDIR_H

#include "../Common/MyString.h"
#include "Defs.h"

/* GetFullPathName for 7zAES.cpp */
DWORD WINAPI GetFullPathName( LPCSTR name, DWORD len, LPSTR buffer, LPSTR *lastpart );

namespace NWindows {
namespace NFile {
namespace NDirectory {

bool SetDirTime(LPCWSTR fileName, const FILETIME *creationTime, const FILETIME *lastAccessTime, const FILETIME *lastWriteTime);

bool MySetFileAttributes(LPCTSTR fileName, DWORD fileAttributes);
#ifndef _UNICODE
bool MySetFileAttributes(LPCWSTR fileName, DWORD fileAttributes);
#endif

bool MyMoveFile(LPCTSTR existFileName, LPCTSTR newFileName);
#ifndef _UNICODE
bool MyMoveFile(LPCWSTR existFileName, LPCWSTR newFileName);
#endif

bool MyRemoveDirectory(LPCTSTR pathName);
#ifndef _UNICODE
bool MyRemoveDirectory(LPCWSTR pathName);
#endif

bool MyCreateDirectory(LPCTSTR pathName);
bool CreateComplexDirectory(LPCTSTR pathName);
#ifndef _UNICODE
bool MyCreateDirectory(LPCWSTR pathName);
bool CreateComplexDirectory(LPCWSTR pathName);
#endif

bool DeleteFileAlways(LPCTSTR name);
#ifndef _UNICODE
bool DeleteFileAlways(LPCWSTR name);
#endif
bool RemoveDirectoryWithSubItems(const UString &path);

#ifndef _WIN32_WCE
bool MyGetFullPathName(LPCTSTR fileName, CSysString &resultPath, 
    int &fileNamePartStartIndex);
bool MyGetFullPathName(LPCTSTR fileName, CSysString &resultPath);
bool GetOnlyName(LPCTSTR fileName, CSysString &resultName);
bool GetOnlyDirPrefix(LPCTSTR fileName, CSysString &resultName);
#ifndef _UNICODE
bool MyGetFullPathName(LPCWSTR fileName, UString &resultPath, 
    int &fileNamePartStartIndex);
bool MyGetFullPathName(LPCWSTR fileName, UString &resultPath);
#endif

#endif

bool MySetCurrentDirectory(LPCWSTR path);
bool MyGetCurrentDirectory(CSysString &resultPath);

bool MySearchPath(LPCWSTR path, LPCWSTR fileName, LPCWSTR extension, UString &resultPath);

bool MyGetTempPath(CSysString &resultPath);
#ifndef _UNICODE
bool MyGetTempPath(UString &resultPath);
#endif

class CTempFile
{
  bool _mustBeDeleted;
  CSysString _fileName;
public:
  CTempFile(): _mustBeDeleted(false) {}
  ~CTempFile() { Remove(); }
  void DisableDeleting() { _mustBeDeleted = false; }
  UINT Create(LPCTSTR dirPath, LPCTSTR prefix, CSysString &resultPath);
  bool Create(LPCTSTR prefix, CSysString &resultPath);
  bool Remove();
};

#ifdef _UNICODE
typedef CTempFile CTempFileW;
#endif

bool CreateTempDirectory(LPCWSTR prefixChars, UString &dirName);

class CTempDirectory
{
  bool _mustBeDeleted;
  CSysString _tempDir;
public:
  const CSysString &GetPath() const { return _tempDir; }
  CTempDirectory(): _mustBeDeleted(false) {}
  ~CTempDirectory() { Remove();  }
  bool Create(LPCTSTR prefix) ;
  bool Remove()
  {
    if (!_mustBeDeleted)
      return true;
    _mustBeDeleted = !RemoveDirectoryWithSubItems(_tempDir);
    return (!_mustBeDeleted);
  }
  void DisableDeleting() { _mustBeDeleted = false; }
};

#ifdef _UNICODE
typedef CTempDirectory CTempDirectoryW;
#endif


}}}

#endif
