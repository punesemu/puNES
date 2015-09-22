// Windows/FileFind.cpp

#include "StdAfx.h"

#include "FileFind.h"
#include "../Common/StringConvert.h"

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#ifdef ENV_HAVE_LSTAT
extern "C"
{

int global_use_lstat=1; // default behaviour : p7zip stores symlinks instead of dumping the files they point to
}
#endif

#define NEED_NAME_WINDOWS_TO_UNIX
#include "myPrivate.h"

// #define TRACEN(u) u;
#define TRACEN(u)  /* */

void my_windows_split_path(const AString &p_path, AString &dir , AString &base) {
  int pos = p_path.ReverseFind('/');
  if (pos == -1) {
    // no separator
    dir  = ".";
    if (p_path.IsEmpty())
      base = ".";
    else
      base = p_path;
  } else if ((pos+1) < p_path.Length()) {
    // true separator
    base = p_path.Mid(pos+1);
    while ((pos >= 1) && (p_path[pos-1] == '/'))
      pos--;
    if (pos == 0)
      dir = "/";
    else
      dir = p_path.Left(pos);
  } else {
    // separator at the end of the path
    // pos = p_path.find_last_not_of("/");
    pos = -1;
    int ind = 0;
    while (p_path[ind]) {
      if (p_path[ind] != '/')
        pos = ind;
      ind++;
    }
    if (pos == -1) {
      base = "/";
      dir = "/";
    } else {
      my_windows_split_path(p_path.Left(pos+1),dir,base);
    }
  }
}

static void my_windows_split_path(const UString &p_path, UString &dir , UString &base) {
  int pos = p_path.ReverseFind(L'/');
  if (pos == -1) {
    // no separator
    dir  = L".";
    if (p_path.IsEmpty())
      base = L".";
    else
      base = p_path;
  } else if ((pos+1) < p_path.Length()) {
    // true separator
    base = p_path.Mid(pos+1);
    while ((pos >= 1) && (p_path[pos-1] == L'/'))
      pos--;
    if (pos == 0)
      dir = L"/";
    else
      dir = p_path.Left(pos);
  } else {
    // separator at the end of the path
    // pos = p_path.find_last_not_of("/");
    pos = -1;
    int ind = 0;
    while (p_path[ind]) {
      if (p_path[ind] != L'/')
        pos = ind;
      ind++;
    }
    if (pos == -1) {
      base = L"/";
      dir = L"/";
    } else {
      my_windows_split_path(p_path.Left(pos+1),dir,base);
    }
  }
}

static int filter_pattern(const char *string , const char *pattern , int flags_nocase) {
  if ((string == 0) || (*string==0)) {
    if (pattern == 0)
      return 1;
    while (*pattern=='*')
      ++pattern;
    return (!*pattern);
  }

  switch (*pattern) {
  case '*':
    if (!filter_pattern(string+1,pattern,flags_nocase))
      return filter_pattern(string,pattern+1,flags_nocase);
    return 1;
  case 0:
    if (*string==0)
      return 1;
    break;
  case '?':
    return filter_pattern(string+1,pattern+1,flags_nocase);
  default:
    if (   ((flags_nocase) && (tolower(*pattern)==tolower(*string)))
           || (*pattern == *string)
       ) {
      return filter_pattern(string+1,pattern+1,flags_nocase);
    }
    break;
  }
  return 0;
}


namespace NWindows {
namespace NFile {
namespace NFind {

static const TCHAR kDot = TEXT('.');

bool CFileInfo::IsDots() const
{ 
  if (!IsDir() || Name.IsEmpty())
    return false;
  if (Name[0] != kDot)
    return false;
  return Name.Length() == 1 || (Name[1] == kDot && Name.Length() == 2);
}

bool CFileInfoW::IsDots() const
{ 
  if (!IsDir() || Name.IsEmpty())
    return false;
  if (Name[0] != kDot)
    return false;
  return Name.Length() == 1 || (Name[1] == kDot && Name.Length() == 2);
}

static bool originalFilename(const UString & src, AString & res)
{
  // Try to recover the original filename
  res = "";
  int i=0;
  while (src[i])
  {
    if (src[i] >= 256) {
      return false;
    } else {
      res += char(src[i]);
    }
    i++;
  }
  return true;
}



// Warning this function cannot update "fileInfo.Name"
static int fillin_CFileInfo(CFileInfo &fileInfo,const char *filename) {
  struct stat stat_info;

  int ret;
#ifdef ENV_HAVE_LSTAT
  if (global_use_lstat) {
    ret = lstat(filename,&stat_info);
  } else
#endif
  {
     ret = stat(filename,&stat_info);
  }

  if (ret != 0) return ret;

  /* FIXME : FILE_ATTRIBUTE_HIDDEN ? */
  if (S_ISDIR(stat_info.st_mode)) {
    fileInfo.Attrib = FILE_ATTRIBUTE_DIRECTORY;
  } else {
    fileInfo.Attrib = FILE_ATTRIBUTE_ARCHIVE;
  }

  if (!(stat_info.st_mode & S_IWUSR))
    fileInfo.Attrib |= FILE_ATTRIBUTE_READONLY;

  fileInfo.Attrib |= FILE_ATTRIBUTE_UNIX_EXTENSION + ((stat_info.st_mode & 0xFFFF) << 16);

  RtlSecondsSince1970ToFileTime( stat_info.st_ctime, &fileInfo.CTime );
  RtlSecondsSince1970ToFileTime( stat_info.st_mtime, &fileInfo.MTime );
  RtlSecondsSince1970ToFileTime( stat_info.st_atime, &fileInfo.ATime );

  fileInfo.IsDevice = false;

  if (S_ISDIR(stat_info.st_mode)) {
    fileInfo.Size = 0;
  } else { // file or symbolic link
    fileInfo.Size = stat_info.st_size; // for a symbolic link, size = size of filename
  }
  return 0;
}

static int fillin_CFileInfo(CFileInfo &fileInfo,const char *dir,const char *name) {
  char filename[MAX_PATHNAME_LEN];
  size_t dir_len = strlen(dir);
  size_t name_len = strlen(name);
  size_t total = dir_len + 1 + name_len + 1; // 1 = strlen("/"); + le zero character
  if (total >= MAX_PATHNAME_LEN) throw "fillin_CFileInfo - internal error - MAX_PATHNAME_LEN";
  memcpy(filename,dir,dir_len);
  if (dir_len >= 1)
  {
	if (filename[dir_len-1] == CHAR_PATH_SEPARATOR)
	{ // delete the '/'
		dir_len--;
	}
  }
  filename[dir_len] = CHAR_PATH_SEPARATOR;
  memcpy(filename+(dir_len+1),name,name_len+1); // copy also final '\0'

  fileInfo.Name = name;

  int ret = fillin_CFileInfo(fileInfo,filename);
  if (ret != 0) {
	AString err_msg = "stat error for ";
        err_msg += filename;
        err_msg += " (";
        err_msg += strerror(errno);
        err_msg += ")";
        throw err_msg;
  }
  return ret;
}

////////////////////////////////
// CFindFile

bool CFindFile::Close()
{

  if(_dirp == 0)
    return true;
  int ret = closedir(_dirp);
  if (ret == 0)
  {
    _dirp = 0;
    return true;
  }
  return false;
}
           
// bool CFindFile::FindFirst(LPCTSTR wildcard, CFileInfo &fileInfo)
bool CFindFile::FindFirst(LPCSTR wildcard, CFileInfo &fileInfo)
{
  if (!Close())
    return false;

  if ((!wildcard) || (wildcard[0]==0)) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return false;
  }
 
  my_windows_split_path(nameWindowToUnix(wildcard),_directory,_pattern);
  
  TRACEN((printf("CFindFile::FindFirst : %s (dirname=%s,pattern=%s)\n",wildcard,(const char *)_directory,(const char *)_pattern)))

  _dirp = ::opendir((const char *)_directory);
  TRACEN((printf("CFindFile::FindFirst : opendir=%p\n",_dirp)))

  if ((_dirp == 0) && (global_use_utf16_conversion)) {
    // Try to recover the original filename
    UString ustr = MultiByteToUnicodeString(_directory, 0);
    AString resultString;
    bool is_good = originalFilename(ustr, resultString);
    if (is_good) {
      _dirp = ::opendir((const char *)resultString);
      _directory = resultString;
    }
  }

  if (_dirp == 0) return false;

  struct dirent *dp;
  while ((dp = readdir(_dirp)) != NULL) {
    if (filter_pattern(dp->d_name,(const char *)_pattern,0) == 1) {
      int retf = fillin_CFileInfo(fileInfo,(const char *)_directory,dp->d_name);
      if (retf)
      {
         TRACEN((printf("CFindFile::FindFirst : closedir-1(dirp=%p)\n",_dirp)))
         closedir(_dirp);
         _dirp = 0;
         SetLastError( ERROR_NO_MORE_FILES );
         return false;
      }
      TRACEN((printf("CFindFile::FindFirst -%s- true\n",dp->d_name)))
      return true;
    }
  }

  TRACEN((printf("CFindFile::FindFirst : closedir-2(dirp=%p)\n",_dirp)))
  closedir(_dirp);
  _dirp = 0;
  SetLastError( ERROR_NO_MORE_FILES );
  return false;
}

bool CFindFile::FindFirst(LPCWSTR wildcard, CFileInfoW &fileInfo)
{
  if (!Close())
    return false;
  CFileInfo fileInfo0;
  AString Awildcard = UnicodeStringToMultiByte(wildcard, CP_ACP);
  bool bret = FindFirst((LPCSTR)Awildcard, fileInfo0);
  if (bret)
  {
     fileInfo.Attrib = fileInfo0.Attrib;
     fileInfo.CTime = fileInfo0.CTime;
     fileInfo.ATime = fileInfo0.ATime;
     fileInfo.MTime = fileInfo0.MTime;
     fileInfo.Size = fileInfo0.Size;
     fileInfo.IsDevice = fileInfo0.IsDevice;
     fileInfo.Name = GetUnicodeString(fileInfo0.Name, CP_ACP);
  }
  return bret;
}

bool CFindFile::FindNext(CFileInfo &fileInfo)
{
  if (_dirp == 0)
  {
    SetLastError( ERROR_INVALID_HANDLE );
    return false;
  }

  struct dirent *dp;
  while ((dp = readdir(_dirp)) != NULL) {
      if (filter_pattern(dp->d_name,(const char *)_pattern,0) == 1) {
        int retf = fillin_CFileInfo(fileInfo,(const char *)_directory,dp->d_name);
        if (retf)
        {
           TRACEN((printf("FindNextFileA -%s- ret_handle=FALSE (errno=%d)\n",dp->d_name,errno)))
           return false;

        }
        TRACEN((printf("FindNextFileA -%s- true\n",dp->d_name)))
        return true;
      }
    }
  TRACEN((printf("FindNextFileA ret_handle=FALSE (ERROR_NO_MORE_FILES)\n")))
  SetLastError( ERROR_NO_MORE_FILES );
  return false;
}

bool CFindFile::FindNext(CFileInfoW &fileInfo)
{
  CFileInfo fileInfo0;
  bool bret = FindNext(fileInfo0);
  if (bret)
  {
     fileInfo.Attrib = fileInfo0.Attrib;
     fileInfo.CTime = fileInfo0.CTime;
     fileInfo.ATime = fileInfo0.ATime;
     fileInfo.MTime = fileInfo0.MTime;
     fileInfo.Size = fileInfo0.Size;
     fileInfo.IsDevice = fileInfo0.IsDevice;
     fileInfo.Name = GetUnicodeString(fileInfo0.Name, CP_ACP);
  }
  return bret;
}

bool CFileInfo::Find(LPCSTR wildcard)
{
  #ifdef SUPPORT_DEVICE_FILE
  if (IsDeviceName(wildcard))
  {
    Clear();
    IsDevice = true;
    NIO::CInFile inFile;
    if (!inFile.Open(wildcard))
      return false;
    Name = wildcard + 4;
    if (inFile.LengthDefined)
      Size = inFile.Length;
    return true;
  }
  #endif
  CFindFile finder;
  return finder.FindFirst(wildcard, *this);
}


// #ifndef _UNICODE
bool CFileInfoW::Find(LPCWSTR wildcard)
{
  #ifdef SUPPORT_DEVICE_FILE
  if (IsDeviceName(wildcard))
  {
    Clear();
    IsDevice = true;
    NIO::CInFile inFile;
    if (!inFile.Open(wildcard))
      return false;
    Name = wildcard + 4;
    if (inFile.LengthDefined)
      Size = inFile.Length;
    return true;
  }
  #endif
  CFindFile finder;
  return finder.FindFirst(wildcard, *this);
}
// #endif

bool FindFile(LPCSTR wildcard, CFileInfo &fileInfo)
{
  // CFindFile finder;
  // return finder.FindFirst(wildcard, fileInfo);
  AString dir,base;
  my_windows_split_path(wildcard, dir , base);
  int ret = fillin_CFileInfo(fileInfo,nameWindowToUnix(wildcard));
  fileInfo.Name = base;
  TRACEN((printf("FindFile(%s,CFileInfo) ret=%d\n",wildcard,ret)))
  return (ret == 0);
}

bool FindFile(LPCWSTR wildcard, CFileInfoW &fileInfo)
{
  // CFindFile finder;
  // return finder.FindFirst(wildcard, fileInfo);
  AString name = UnicodeStringToMultiByte(wildcard, CP_ACP); 
  CFileInfo fileInfo0;
  int ret = fillin_CFileInfo(fileInfo0,nameWindowToUnix((const char *)name));
  TRACEN((printf("FindFile-1(%s,CFileInfo) ret=%d\n",(const char *)name,ret)))
  if (ret != 0)
  {
    // Try to recover the original filename
    AString resultString;
    bool is_good = originalFilename(wildcard, resultString);
    if (is_good) {
       ret = fillin_CFileInfo(fileInfo0,nameWindowToUnix((const char *)resultString));
       TRACEN((printf("FindFile-2(%s,CFileInfo) ret=%d\n",(const char *)resultString,ret)))
    }
  }
  if (ret == 0)
  {
     UString dir,base;
     my_windows_split_path(wildcard, dir , base);
     fileInfo.Attrib = fileInfo0.Attrib;
     fileInfo.CTime = fileInfo0.CTime;
     fileInfo.ATime = fileInfo0.ATime;
     fileInfo.MTime = fileInfo0.MTime;
     fileInfo.Size = fileInfo0.Size;
     fileInfo.Name = base;
  }
  return (ret == 0);
}

bool DoesFileExist(LPCSTR name) // FIXME
{
  CFileInfo fi;
  int ret = fillin_CFileInfo(fi,nameWindowToUnix(name));
  TRACEN((printf("DoesFileExist(%s) ret=%d\n",name,ret)))
  return (ret == 0) && !fi.IsDir();;
}

bool DoesDirExist(LPCSTR name) // FIXME
{
  CFileInfo fi;
  int ret = fillin_CFileInfo(fi,nameWindowToUnix(name));
  TRACEN((printf("DoesDirExist(%s) ret=%d\n",name,ret)))
  return (ret == 0) && fi.IsDir();;
}

bool DoesFileOrDirExist(LPCSTR name)
{
  CFileInfo fileInfo;
  int ret = fillin_CFileInfo(fileInfo,nameWindowToUnix(name));
  TRACEN((printf("DoesFileOrDirExist(%s) ret=%d\n",name,ret)))
  return (ret == 0);
}

bool DoesFileExist(LPCWSTR name)
{
  AString Aname = UnicodeStringToMultiByte(name, CP_ACP); 
  bool bret = DoesFileExist((LPCSTR)Aname);
  if (bret) return bret;

  // Try to recover the original filename
  AString resultString;
  bool is_good = originalFilename(name, resultString);
  if (is_good) {
     bret = DoesFileExist((const char *)resultString);
  }
  return bret;
}

bool DoesDirExist(LPCWSTR name)
{
  AString Aname = UnicodeStringToMultiByte(name, CP_ACP); 
  bool bret = DoesDirExist((LPCSTR)Aname);
  if (bret) return bret;

  // Try to recover the original filename
  AString resultString;
  bool is_good = originalFilename(name, resultString);
  if (is_good) {
     bret = DoesDirExist((const char *)resultString);
  }
  return bret;
}

bool DoesFileOrDirExist(LPCWSTR name)
{
  AString Aname = UnicodeStringToMultiByte(name, CP_ACP); 
  bool bret = DoesFileOrDirExist((LPCSTR)Aname);
  if (bret) return bret;

  // Try to recover the original filename
  AString resultString;
  bool is_good = originalFilename(name, resultString);
  if (is_good) {
     bret = DoesFileOrDirExist((const char *)resultString);
  }
  return bret;
}

/////////////////////////////////////
// CEnumerator

bool CEnumerator::NextAny(CFileInfo &fileInfo)
{
  if(_findFile.IsHandleAllocated())
    return _findFile.FindNext(fileInfo);
  else
    return _findFile.FindFirst(_wildcard, fileInfo);
}

bool CEnumerator::Next(CFileInfo &fileInfo)
{
  while(true)
  {
    if(!NextAny(fileInfo))
      return false;
    if(!fileInfo.IsDots())
      return true;
  }
}

bool CEnumerator::Next(CFileInfo &fileInfo, bool &found)
{
  if (Next(fileInfo))
  {
    found = true;
    return true;
  }
  found = false;
  return (::GetLastError() == ERROR_NO_MORE_FILES);
}

bool CEnumeratorW::NextAny(CFileInfoW &fileInfo)
{
  if(_findFile.IsHandleAllocated())
    return _findFile.FindNext(fileInfo);
  else
    return _findFile.FindFirst(_wildcard, fileInfo);
}

bool CEnumeratorW::Next(CFileInfoW &fileInfo)
{
  while(true)
  {
    if(!NextAny(fileInfo))
      return false;
    if(!fileInfo.IsDots())
      return true;
  }
}

bool CEnumeratorW::Next(CFileInfoW &fileInfo, bool &found)
{
  if (Next(fileInfo))
  {
    found = true;
    return true;
  }
  found = false;
  return (::GetLastError() == ERROR_NO_MORE_FILES);
}


}}}
