// Windows/FileDir.cpp

#include "StdAfx.h"

#include "FileDir.h"
#include "FileName.h"
#include "FileFind.h"
#include "Defs.h"
#include "../Common/StringConvert.h"
#include "../Common/IntToString.h"

#define NEED_NAME_WINDOWS_TO_UNIX
#include "myPrivate.h"
#include "Windows/Synchronization.h"

#include <unistd.h> // rmdir
#include <errno.h>

#include <sys/stat.h> // mkdir
#include <sys/types.h>
#include <fcntl.h>

#include <utime.h>

// #define TRACEN(u) u;
#define TRACEN(u)  /* */

class Umask
{
  public:
  mode_t  current_umask;
  mode_t  mask;
  Umask() {
    current_umask = umask (0);  /* get and set the umask */
    umask(current_umask);	/* restore the umask */
    mask = 0777 & (~current_umask);
  } 
};

static Umask gbl_umask;

#ifdef _UNICODE
AString nameWindowToUnix2(LPCWSTR name) // FIXME : optimization ?
{
   AString astr = UnicodeStringToMultiByte(name);
   return AString(nameWindowToUnix((const char *)astr));
}
#endif

extern BOOLEAN WINAPI RtlTimeToSecondsSince1970( const LARGE_INTEGER *Time, DWORD *Seconds );

#ifdef _UNICODE
DWORD WINAPI GetFullPathName( LPCTSTR name, DWORD len, LPTSTR buffer, LPTSTR *lastpart ) { // FIXME
  if (name == 0) return 0;

  DWORD name_len = lstrlen(name);

  if (name[0] == '/') {
    DWORD ret = name_len+2;
    if (ret >= len) {
      TRACEN((printf("GetFullPathNameA(%ls,%d,)=0000 (case 0)\n",name, (int)len)))
      return 0;
    }
    lstrcpy(buffer,L"c:");
    lstrcat(buffer,name);

    *lastpart=buffer;
    TCHAR *ptr=buffer;
    while (*ptr) {
      if (*ptr == '/')
        *lastpart=ptr+1;
      ptr++;
    }
    TRACEN((printf("GetFullPathNameA(%s,%d,%ls,%ls)=%d\n",name, (int)len,buffer, *lastpart,(int)ret)))
    return ret;
  }
  if (isascii(name[0]) && (name[1] == ':')) { // FIXME isascii
    DWORD ret = name_len;
    if (ret >= len) {
      TRACEN((printf("GetFullPathNameA(%ls,%d,)=0000 (case 1)\n",name, (int)len)))
      return 0;
    }
    lstrcpy(buffer,name);

    *lastpart=buffer;
    TCHAR *ptr=buffer;
    while (*ptr) {
      if (*ptr == '/')
        *lastpart=ptr+1;
      ptr++;
    }
    TRACEN((printf("GetFullPathNameA(%sl,%d,%ls,%ls)=%d\n",name, (int)len,buffer, *lastpart,(int)ret)))
    return ret;
  }

  // name is a relative pathname.
  //
  if (len < 2) {
    TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 2)\n",name, (int)len)))
    return 0;
  }

  DWORD ret = 0;
  char begin[MAX_PATHNAME_LEN];
  /* DWORD begin_len = GetCurrentDirectoryA(MAX_PATHNAME_LEN,begin); */
  DWORD begin_len = 0;
  begin[0]='c';
  begin[1]=':';
  char * cret = getcwd(begin+2, MAX_PATHNAME_LEN - 3);
  if (cret) {
    begin_len = strlen(begin);
  }
   
  if (begin_len >= 1) {
    //    strlen(begin) + strlen("/") + strlen(name)
    ret = begin_len     +    1        + name_len;

    if (ret >= len) {
      TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 4)\n",name, (int)len)))
      return 0;
    }
    UString wbegin = GetUnicodeString(begin);
    lstrcpy(buffer,wbegin);
    lstrcat(buffer,L"/");
    lstrcat(buffer,name);

    *lastpart=buffer + begin_len + 1;
    TCHAR *ptr=buffer;
    while (*ptr) {
      if (*ptr == '/')
        *lastpart=ptr+1;
      ptr++;
    }
    TRACEN((printf("GetFullPathNameA(%s,%d,%s,%s)=%d\n",name, (int)len,buffer, *lastpart,(int)ret)))
  } else {
    ret = 0;
    TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 5)\n",name, (int)len)))
  }
  return ret;
}

#endif

#if 0
DWORD WINAPI GetFullPathName( LPCSTR name, DWORD len, LPSTR buffer, LPSTR *lastpart ) {
  if (name == 0) return 0;

  DWORD name_len = strlen(name);

  if (name[0] == '/') {
    DWORD ret = name_len+2;
    if (ret >= len) {
      TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 0)\n",name, (int)len)))
      return 0;
    }
    strcpy(buffer,"c:");
    strcat(buffer,name);

    *lastpart=buffer;
    char *ptr=buffer;
    while (*ptr) {
      if (*ptr == '/')
        *lastpart=ptr+1;
      ptr++;
    }
    TRACEN((printf("GetFullPathNameA(%s,%d,%s,%s)=%d\n",name, (int)len,buffer, *lastpart,(int)ret)))
    return ret;
  }
  if (isascii(name[0]) && (name[1] == ':')) {
    DWORD ret = name_len;
    if (ret >= len) {
      TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 1)\n",name, (int)len)))
      return 0;
    }
    strcpy(buffer,name);

    *lastpart=buffer;
    char *ptr=buffer;
    while (*ptr) {
      if (*ptr == '/')
        *lastpart=ptr+1;
      ptr++;
    }
    TRACEN((printf("GetFullPathNameA(%s,%d,%s,%s)=%d\n",name, (int)len,buffer, *lastpart,(int)ret)))
    return ret;
  }

  // name is a relative pathname.
  //
  if (len < 2) {
    TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 2)\n",name, (int)len)))
    return 0;
  }

  DWORD ret = 0;
  char begin[MAX_PATHNAME_LEN];
  /* DWORD begin_len = GetCurrentDirectoryA(MAX_PATHNAME_LEN,begin); */
  DWORD begin_len = 0;
  begin[0]='c';
  begin[1]=':';
  char * cret = getcwd(begin+2, MAX_PATHNAME_LEN - 3);
  if (cret) {
    begin_len = strlen(begin);
  }
   
  if (begin_len >= 1) {
    //    strlen(begin) + strlen("/") + strlen(name)
    ret = begin_len     +    1        + name_len;

    if (ret >= len) {
      TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 4)\n",name, (int)len)))
      return 0;
    }
    strcpy(buffer,begin);
    strcat(buffer,"/");
    strcat(buffer,name);

    *lastpart=buffer + begin_len + 1;
    char *ptr=buffer;
    while (*ptr) {
      if (*ptr == '/')
        *lastpart=ptr+1;
      ptr++;
    }
    TRACEN((printf("GetFullPathNameA(%s,%d,%s,%s)=%d\n",name, (int)len,buffer, *lastpart,(int)ret)))
  } else {
    ret = 0;
    TRACEN((printf("GetFullPathNameA(%s,%d,)=0000 (case 5)\n",name, (int)len)))
  }
  return ret;
}

static BOOL WINAPI RemoveDirectory(LPCSTR path) {
  if (!path || !*path) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return FALSE;
  }
  const char * name = nameWindowToUnix(path);
  TRACEN((printf("RemoveDirectoryA(%s)\n",name)))

  if (rmdir( name ) != 0) {
    return FALSE;
  }
  return TRUE;
}
#endif

#ifdef _UNICODE
static BOOL WINAPI RemoveDirectory(LPCWSTR path) {
  if (!path || !*path) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return FALSE;
  }
  AString name = nameWindowToUnix2(path);
  TRACEN((printf("RemoveDirectoryA(%s)\n",(const char *)name)))

  if (rmdir( (const char *)name ) != 0) {
    return FALSE;
  }
  return TRUE;
}
#endif

static int copy_fd(int fin,int fout)
{
  char buffer[16384];
  ssize_t ret_in;
  ssize_t ret_out;

  do {
    ret_out = -1;
    do {
      ret_in = read(fin, buffer,sizeof(buffer));
    } while (ret_in < 0 && (errno == EINTR));
    if (ret_in >= 1) {
      do {
        ret_out = write (fout, buffer, ret_in);
      } while (ret_out < 0 && (errno == EINTR));
    } else if (ret_in == 0) {
      ret_out = 0;
    }
  } while (ret_out >= 1);
  return ret_out;
}

static BOOL CopyFile(const char *src,const char *dst)
{
  int ret = -1;

#ifdef O_BINARY
  int   flags = O_BINARY;
#else
  int   flags = 0;
#endif

#ifdef O_LARGEFILE
  flags |= O_LARGEFILE;
#endif

  int fout = open(dst,O_CREAT | O_WRONLY | O_EXCL | flags, 0600);
  if (fout != -1)
  {
    int fin = open(src,O_RDONLY | flags , 0600);
    if (fin != -1)
    {
      ret = copy_fd(fin,fout);
      if (ret == 0) ret = close(fin);
      else                close(fin);
    }
    if (ret == 0) ret = close(fout);
    else                close(fout);
  }
  if (ret == 0) return TRUE;
  return FALSE;
}

/*****************************************************************************************/


namespace NWindows {
namespace NFile {
namespace NDirectory {


bool MySetCurrentDirectory(LPCWSTR wpath)
{
   AString path = UnicodeStringToMultiByte(wpath);

   return chdir((const char*)path) == 0;
}

#ifdef _UNICODE
bool GetOnlyName(LPCTSTR fileName, CSysString &resultName)
{
  int index;
  if (!MyGetFullPathName(fileName, resultName, index))
    return false;
  resultName = resultName.Mid(index);
  return true;
}

bool GetOnlyDirPrefix(LPCTSTR fileName, CSysString &resultName)
{
  int index;
  if (!MyGetFullPathName(fileName, resultName, index))
    return false;
  resultName = resultName.Left(index);
  return true;
}
#endif


bool MyGetCurrentDirectory(CSysString &resultPath)
{
  char begin[MAX_PATHNAME_LEN];
  begin[0]='c';
  begin[1]=':';
  char * cret = getcwd(begin+2, MAX_PATHNAME_LEN - 3);
  if (cret)
  {
#ifdef _UNICODE
    resultPath = GetUnicodeString(begin);
#else
    resultPath = begin;
#endif
    return true;
  }
  return false;
}

bool MyMoveFile( LPCTSTR fn1, LPCTSTR fn2 ) {
#ifdef _UNICODE
  AString src = nameWindowToUnix2(fn1);
  AString dst = nameWindowToUnix2(fn2);
#else
  const char * src = nameWindowToUnix(fn1);
  const char * dst = nameWindowToUnix(fn2);
#endif

  TRACEN((printf("MoveFileW(%s,%s)\n",src,dst)))

  int ret = rename(src,dst);
  if (ret != 0)
  {
    if (errno == EXDEV) // FIXED : bug #1112167 (Temporary directory must be on same partition as target)
    {
      BOOL bret = CopyFile(src,dst);
      if (bret == FALSE) return false;

      struct stat info_file;
      ret = stat(src,&info_file);
      if (ret == 0) {
	TRACEN((printf("##DBG chmod-1(%s,%o)\n",dst,(unsigned)info_file.st_mode & gbl_umask.mask)))
        ret = chmod(dst,info_file.st_mode & gbl_umask.mask);
      }
      if (ret == 0) {
         ret = unlink(src);
      }
      if (ret == 0) return true;
    }
    return false;
  }
  return true;
}

bool MyRemoveDirectory(LPCTSTR pathName)
{
  return BOOLToBool(::RemoveDirectory(pathName));
}

bool SetDirTime(LPCWSTR fileName, const FILETIME * /* creationTime */ ,
      const FILETIME *lpLastAccessTime, const FILETIME *lpLastWriteTime)
{
  AString  cfilename = UnicodeStringToMultiByte(fileName);
  const char * unix_filename = nameWindowToUnix((const char *)cfilename);

  struct utimbuf buf;

  struct stat    oldbuf;
  int ret = stat(unix_filename,&oldbuf);
  if (ret == 0) {
    buf.actime  = oldbuf.st_atime;
    buf.modtime = oldbuf.st_mtime;
  } else {
    time_t current_time = time(0);
    buf.actime  = current_time;
    buf.modtime = current_time;
  }

  if (lpLastAccessTime)
  {
    LARGE_INTEGER  ltime;
    DWORD dw;
    ltime.QuadPart = lpLastAccessTime->dwHighDateTime;
    ltime.QuadPart = (ltime.QuadPart << 32) | lpLastAccessTime->dwLowDateTime;
    RtlTimeToSecondsSince1970( &ltime, &dw );
    buf.actime = dw;
  }

  if (lpLastWriteTime)
  {
    LARGE_INTEGER  ltime;
    DWORD dw;
    ltime.QuadPart = lpLastWriteTime->dwHighDateTime;
    ltime.QuadPart = (ltime.QuadPart << 32) | lpLastWriteTime->dwLowDateTime;
    RtlTimeToSecondsSince1970( &ltime, &dw );
    buf.modtime = dw;
  }

  /* ret = */ utime(unix_filename, &buf);

  return true;
}

#ifndef _UNICODE
bool MySetFileAttributes(LPCWSTR fileName, DWORD fileAttributes)
{  
  return MySetFileAttributes(UnicodeStringToMultiByte(fileName, CP_ACP), fileAttributes);
}

bool MyRemoveDirectory(LPCWSTR pathName)
{  
  return MyRemoveDirectory(UnicodeStringToMultiByte(pathName, CP_ACP));
}

bool MyMoveFile(LPCWSTR existFileName, LPCWSTR newFileName)
{  
  UINT codePage = CP_ACP;
  return MyMoveFile(UnicodeStringToMultiByte(existFileName, codePage), UnicodeStringToMultiByte(newFileName, codePage));
}
#endif


static int convert_to_symlink(const char * name) {
  FILE *file = fopen(name,"rb");
  if (file) {
    char buf[MAX_PATHNAME_LEN+1];
    char * ret = fgets(buf,sizeof(buf)-1,file);
    fclose(file);
    if (ret) {
      int ir = unlink(name);
      if (ir == 0) {
        ir = symlink(buf,name);
      }
      return ir;    
    }
  }
  return -1;
}

bool MySetFileAttributes(LPCTSTR fileName, DWORD fileAttributes)
{
  if (!fileName) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    TRACEN((printf("MySetFileAttributes(NULL,%d) : false-1\n",fileAttributes)))
    return false;
  }
#ifdef _UNICODE
  AString name = nameWindowToUnix2(fileName);
#else
  const char * name = nameWindowToUnix(fileName);
#endif
  struct stat stat_info;
#ifdef ENV_HAVE_LSTAT
  if (global_use_lstat) {
    if(lstat(name,&stat_info)!=0) {
      TRACEN((printf("MySetFileAttributes(%s,%d) : false-2-1\n",name,fileAttributes)))
      return false;
    }
  } else
#endif
  {
    if(stat(name,&stat_info)!=0) {
      TRACEN((printf("MySetFileAttributes(%s,%d) : false-2-2\n",name,fileAttributes)))
      return false;
    }
  }

  if (fileAttributes & FILE_ATTRIBUTE_UNIX_EXTENSION) {
     stat_info.st_mode = fileAttributes >> 16;
#ifdef ENV_HAVE_LSTAT
     if (S_ISLNK(stat_info.st_mode)) {
        if ( convert_to_symlink(name) != 0) {
          TRACEN((printf("MySetFileAttributes(%s,%d) : false-3\n",name,fileAttributes)))
          return false;
        }
     } else
#endif
     if (S_ISREG(stat_info.st_mode)) {
       TRACEN((printf("##DBG chmod-2(%s,%o)\n",name,(unsigned)stat_info.st_mode & gbl_umask.mask)))
       chmod(name,stat_info.st_mode & gbl_umask.mask);
     } else if (S_ISDIR(stat_info.st_mode)) {
       // user/7za must be able to create files in this directory
       stat_info.st_mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
       TRACEN((printf("##DBG chmod-3(%s,%o)\n",name,(unsigned)stat_info.st_mode & gbl_umask.mask)))
       chmod(name,stat_info.st_mode & gbl_umask.mask);
     }
#ifdef ENV_HAVE_LSTAT
  } else if (!S_ISLNK(stat_info.st_mode)) {
    // do not use chmod on a link
#else
  } else {
#endif

    /* Only Windows Attributes */
    if( S_ISDIR(stat_info.st_mode)) {
       /* Remark : FILE_ATTRIBUTE_READONLY ignored for directory. */
       TRACEN((printf("##DBG chmod-4(%s,%o)\n",name,(unsigned)stat_info.st_mode & gbl_umask.mask)))
       chmod(name,stat_info.st_mode & gbl_umask.mask);
    } else {
       if (fileAttributes & FILE_ATTRIBUTE_READONLY) stat_info.st_mode &= ~0222; /* octal!, clear write permission bits */
       TRACEN((printf("##DBG chmod-5(%s,%o)\n",name,(unsigned)stat_info.st_mode & gbl_umask.mask)))
       chmod(name,stat_info.st_mode & gbl_umask.mask);
    }
  }
  TRACEN((printf("MySetFileAttributes(%s,%d) : true\n",name,fileAttributes)))

  return true;
}

bool MyCreateDirectory(LPCTSTR pathName)
{  
  if (!pathName || !*pathName) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return false;
  }

#ifdef _UNICODE
  AString name = nameWindowToUnix2(pathName);
#else
  const char * name = nameWindowToUnix(pathName);
#endif
  bool bret = false;
  if (mkdir( name, 0700 ) == 0) bret = true;

  TRACEN((printf("MyCreateDirectory(%s)=%d\n",name,(int)bret)))
  return bret;
}

#ifndef _UNICODE
bool MyCreateDirectory(LPCWSTR pathName)
{  
  return MyCreateDirectory(UnicodeStringToMultiByte(pathName, CP_ACP));
}
#endif

bool CreateComplexDirectory(LPCTSTR _aPathName)
{
  CSysString pathName = _aPathName;
  int pos = pathName.ReverseFind(TEXT(CHAR_PATH_SEPARATOR));
  if (pos > 0 && pos == pathName.Length() - 1)
  {
    if (pathName.Length() == 3 && pathName[1] == ':')
      return true; // Disk folder;
    pathName.Delete(pos);
  }
  CSysString pathName2 = pathName;
  pos = pathName.Length();
  while(true)
  {
    if(MyCreateDirectory(pathName))
      break;
    if(::GetLastError() == ERROR_ALREADY_EXISTS)
    {
#ifdef _WIN32 // FIXED for supporting symbolic link instead of a directory
      NFind::CFileInfo fileInfo;
      if (!NFind::FindFile(pathName, fileInfo)) // For network folders
        return true;
      if (!fileInfo.IsDir())
        return false;
#endif
      break;
    }
    pos = pathName.ReverseFind(TEXT(CHAR_PATH_SEPARATOR));
    if (pos < 0 || pos == 0)
      return false;
    if (pathName[pos - 1] == ':')
      return false;
    pathName = pathName.Left(pos);
  }
  pathName = pathName2;
  while(pos < pathName.Length())
  {
    pos = pathName.Find(TEXT(CHAR_PATH_SEPARATOR), pos + 1);
    if (pos < 0)
      pos = pathName.Length();
    if(!MyCreateDirectory(pathName.Left(pos)))
      return false;
  }
  return true;
}

#ifndef _UNICODE

bool CreateComplexDirectory(LPCWSTR _aPathName)
{
  UString pathName = _aPathName;
  int pos = pathName.ReverseFind(WCHAR_PATH_SEPARATOR);
  if (pos > 0 && pos == pathName.Length() - 1)
  {
    if (pathName.Length() == 3 && pathName[1] == L':')
      return true; // Disk folder;
    pathName.Delete(pos);
  }
  UString pathName2 = pathName;
  pos = pathName.Length();
  while(true)
  {
    if(MyCreateDirectory(pathName))
      break;
    if(::GetLastError() == ERROR_ALREADY_EXISTS)
    {
#ifdef _WIN32 // FIXED for supporting symbolic link instead of a directory
      NFind::CFileInfoW fileInfo;
      if (!NFind::FindFile(pathName, fileInfo)) // For network folders
        return true;
      if (!fileInfo.IsDir())
        return false;
#endif
      break;
    }
    pos = pathName.ReverseFind(WCHAR_PATH_SEPARATOR);
    if (pos < 0 || pos == 0)
      return false;
    if (pathName[pos - 1] == L':')
      return false;
    pathName = pathName.Left(pos);
  }
  pathName = pathName2;
  while(pos < pathName.Length())
  {
    pos = pathName.Find(WCHAR_PATH_SEPARATOR, pos + 1);
    if (pos < 0)
      pos = pathName.Length();
    if(!MyCreateDirectory(pathName.Left(pos)))
      return false;
  }
  return true;
}

#endif

bool DeleteFileAlways(LPCTSTR name)
{
  if (!name || !*name) {
    SetLastError(ERROR_PATH_NOT_FOUND);
    return false;
  }
#ifdef _UNICODE
   AString unixname = nameWindowToUnix2(name);
#else
   const char * unixname = nameWindowToUnix(name);
#endif
   bool bret = false;
   if (remove(unixname) == 0) bret = true;
   TRACEN((printf("DeleteFileAlways(%s)=%d\n",unixname,(int)bret)))
   return bret;
}

#ifndef _UNICODE
bool DeleteFileAlways(LPCWSTR name)
{  
  return DeleteFileAlways(UnicodeStringToMultiByte(name, CP_ACP));
}
#endif


static bool RemoveDirectorySubItems2(const UString &pathPrefix, const NFind::CFileInfoW &fileInfo)
{
  if (fileInfo.IsDir())
    return RemoveDirectoryWithSubItems(pathPrefix + fileInfo.Name);
  return DeleteFileAlways(pathPrefix + fileInfo.Name);
}

bool RemoveDirectoryWithSubItems(const UString &path)
{
  NFind::CFileInfoW fileInfo;
  UString pathPrefix = path + NName::kDirDelimiter;
  {
    NFind::CEnumeratorW enumerator(pathPrefix + TCHAR(NName::kAnyStringWildcard));
    while (enumerator.Next(fileInfo))
      if (!RemoveDirectorySubItems2(pathPrefix, fileInfo))
        return false;
  }
  if (!MySetFileAttributes(path, 0))
    return false;
  return MyRemoveDirectory(path);
}

#ifndef _WIN32_WCE

bool MyGetFullPathName(LPCTSTR fileName, CSysString &resultPath, 
    int &fileNamePartStartIndex)
{
  LPTSTR fileNamePointer = 0;
  LPTSTR buffer = resultPath.GetBuffer(MAX_PATH);
  DWORD needLength = ::GetFullPathName(fileName, MAX_PATH + 1, 
      buffer, &fileNamePointer);
  resultPath.ReleaseBuffer();
  if (needLength == 0 || needLength >= MAX_PATH)
    return false;
  if (fileNamePointer == 0)
    fileNamePartStartIndex = lstrlen(fileName);
  else
    fileNamePartStartIndex = fileNamePointer - buffer;
  return true;
}

#ifndef _UNICODE
bool MyGetFullPathName(LPCWSTR fileName, UString &resultPath, 
    int &fileNamePartStartIndex)
{
    const UINT currentPage = CP_ACP;
    CSysString sysPath;
    if (!MyGetFullPathName(UnicodeStringToMultiByte(fileName, 
        currentPage), sysPath, fileNamePartStartIndex))
      return false;
    UString resultPath1 = MultiByteToUnicodeString(
        sysPath.Left(fileNamePartStartIndex), currentPage);
    UString resultPath2 = MultiByteToUnicodeString(
        sysPath.Mid(fileNamePartStartIndex), currentPage);
    fileNamePartStartIndex = resultPath1.Length();
    resultPath = resultPath1 + resultPath2;
    return true;
}
#endif


bool MyGetFullPathName(LPCTSTR fileName, CSysString &path)
{
  int index;
  return MyGetFullPathName(fileName, path, index);
}

#ifndef _UNICODE
bool MyGetFullPathName(LPCWSTR fileName, UString &path)
{
  int index;
  return MyGetFullPathName(fileName, path, index);
}
#endif

#endif

/* needed to find .DLL/.so and SFX */
bool MySearchPath(LPCWSTR path, LPCWSTR fileName, LPCWSTR extension, UString &resultPath)
{
  if (path != 0) {
    printf("NOT EXPECTED : MySearchPath : path != NULL\n");
    exit(EXIT_FAILURE);
  }

  if (extension != 0) {
    printf("NOT EXPECTED : MySearchPath : extension != NULL\n");
    exit(EXIT_FAILURE);
  }

  if (fileName == 0) {
    printf("NOT EXPECTED : MySearchPath : fileName == NULL\n");
    exit(EXIT_FAILURE);
  }

  const char *p7zip_home_dir = getenv("P7ZIP_HOME_DIR");
  if (p7zip_home_dir) {
    AString file_path = p7zip_home_dir;
    file_path += UnicodeStringToMultiByte(fileName, CP_ACP);

    TRACEN((printf("MySearchPath() fopen(%s)\n",(const char *)file_path)))
    FILE *file = fopen((const char *)file_path,"r");
    if (file) {
      // file is found
      fclose(file);
      resultPath = MultiByteToUnicodeString(file_path, CP_ACP);
      return true;
    }
  }
  return false;
}

#ifndef _UNICODE
bool MyGetTempPath(CSysString &path)
{
  path = "c:/tmp/"; // final '/' is needed
  return true;
}
#endif

bool MyGetTempPath(UString &path)
{
  path = L"c:/tmp/"; // final '/' is needed
  return true;
}

static NSynchronization::CCriticalSection g_CountCriticalSection;

static CSysString CSysConvertUInt32ToString(UInt32 value)
{
  TCHAR buffer[32];
  ConvertUInt32ToString(value, buffer);
  return buffer;
}

UINT CTempFile::Create(LPCTSTR dirPath, LPCTSTR prefix, CSysString &resultPath)
{
  static UInt32 memo_count = 0;
  UInt32 count;

  g_CountCriticalSection.Enter();
  count = memo_count++;
  g_CountCriticalSection.Leave();

  Remove();
/* UINT number = ::GetTempFileName(dirPath, prefix, 0, path.GetBuffer(MAX_PATH)); */
  UINT number = (UINT)getpid();

  resultPath  = dirPath;
  resultPath += prefix;
  resultPath += TEXT('#');
  resultPath += CSysConvertUInt32ToString(number);
  resultPath += TEXT('@');
  resultPath += CSysConvertUInt32ToString(count);
  resultPath += TEXT(".tmp");
  
  _fileName = resultPath;
  _mustBeDeleted = true;
 
  return number;
}

bool CTempFile::Create(LPCTSTR prefix, CSysString &resultPath)
{
  CSysString tempPath;
  if (!MyGetTempPath(tempPath))
    return false;
  if (Create(tempPath, prefix, resultPath) != 0)
    return true;
  return false;
}


bool CTempFile::Remove()
{
  if (!_mustBeDeleted)
    return true;
  _mustBeDeleted = !DeleteFileAlways(_fileName);
  return !_mustBeDeleted;
}

bool CreateTempDirectory(LPCWSTR prefix, UString &dirName)
{
  /*
  CSysString prefix = tempPath + prefixChars;
  CRandom random;
  random.Init();
  */
  for (;;)
  {
    {
      CTempFileW tempFile;
      if (!tempFile.Create(prefix, dirName))
        return false;
      if (!tempFile.Remove())
        return false;
    }
    /*
    UINT32 randomNumber = random.Generate();
    TCHAR randomNumberString[32];
    _stprintf(randomNumberString, _T("%04X"), randomNumber);
    dirName = prefix + randomNumberString;
    */
    if (NFind::DoesFileOrDirExist(dirName))
      continue;
    if (MyCreateDirectory(dirName))
      return true;
    if (::GetLastError() != ERROR_ALREADY_EXISTS)
      return false;
  }
}

bool CTempDirectory::Create(LPCTSTR prefix)
{
  Remove();
  return (_mustBeDeleted = CreateTempDirectory(prefix, _tempDir));
}


}}}
