#if !defined(_WIN32) && !defined(OS2)
#include "lib7zip.h"

#ifdef S_OK
#undef S_OK
#endif

#include "CPP/7zip/Archive/IArchive.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Common/MyCom.h"
#include "CPP/7zip/ICoder.h"
#include "CPP/7zip/IPassword.h"
#include "Common/ComTry.h"
#include "Windows/PropVariant.h"
using namespace NWindows;

#include "HelperFuncs.h"
#include "7ZipFunctions.h"
#include "7ZipDllHandler.h"

#include <stdlib.h>
#include <errno.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "dlfcn.h"
#include "unistd.h"
#include "dirent.h"

#include "OSFunctions_UnixLike.h"

#ifdef __APPLE__
int myselect(struct dirent * pDir );
#else
int myselect(const struct dirent * pDir );
#endif //__APPLE__

static C7ZipObjectPtrArray * g_pHandlers = NULL;
static C7ZipLibrary * g_pLibrary = NULL;

bool LoadDllFromFolder(C7ZipDllHandler * pMainHandler, const wstring & wfolder_name, C7ZipObjectPtrArray & handlers)
{
  g_pHandlers = &handlers;
  g_pLibrary = pMainHandler->GetLibrary();

  string folder_name = NarrowString(wfolder_name);
  string mainHandlerPath = NarrowString(pMainHandler->GetHandlerPath());
  string folderPath = mainHandlerPath + "/" + folder_name;

  char * current_dir = getcwd(NULL, 0);

  int result = chdir(folderPath.c_str());

  struct dirent **namelist = NULL;

  if (result == 0) {
    scandir( ".", &namelist,myselect,alphasort );
  }

  result = chdir(current_dir);

  free(current_dir);

  g_pHandlers = NULL;
  g_pLibrary = NULL;
  return true;
}

#ifdef __APPLE__
int myselect(struct dirent * pDir )
#else
int myselect(const struct dirent * pDir )
#endif //__APPLE__
{
  if ( NULL == pDir )
    return 0;

  const char * szEntryName = pDir->d_name;

  if ( ( strcasecmp( szEntryName,"." ) == 0 ) ||
       ( strcasecmp( szEntryName,".." ) == 0 ) )
  {
    return 0;
  }

  DIR * pTmpDir = NULL;

  if ( NULL == ( pTmpDir = opendir(szEntryName) ) )
  {
    if ( errno == ENOTDIR )
    {
      char * current_path = getcwd(NULL, 0);
      string path = current_path;
      path += "/";
      path += szEntryName;
      free(current_path);

      void * pHandler = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);

      if (pHandler != NULL)
      {
        C7ZipDllHandler * p7ZipHandler = new C7ZipDllHandler(g_pLibrary, pHandler);

        if (p7ZipHandler->IsInitialized())
        {
          g_pHandlers->push_back(p7ZipHandler);
        }
        else
        {
          delete p7ZipHandler;
        }
      }
    }
  }
  else
  {
    closedir( pTmpDir );

    int result = chdir( szEntryName );

    struct dirent **namelist = NULL;

    scandir( ".",&namelist,myselect,alphasort );

    result = chdir( ".." );
  }

  return 0;
}

wstring GetHandlerPath(void * pHandler)
{
  Dl_info info;

  memset(&info, 0, sizeof(Dl_info));

  if (dladdr((void *)pHandler,&info))
  {
    if (info.dli_fname != NULL)
    {
      string path = info.dli_fname;

      size_t pos = path.rfind("/");

      if (pos != string::npos)
      {
        return WidenString(path.substr(0, pos));
      }
    }
  }

  return L".";
}

HMODULE Load7ZLibrary(const wstring & name)
{
  string tmpName = NarrowString(name + L".so");

  HMODULE pHandler = dlopen(tmpName.c_str(), RTLD_LAZY | RTLD_GLOBAL);

  if (pHandler)
    return pHandler;

  size_t pos = tmpName.rfind("/");

  if (pos != string::npos)
  {
    tmpName = tmpName.substr(pos + 1);
  }

  std::vector<const char *> lib_search_pathlist;

  lib_search_pathlist.push_back("/usr/local/lib");
  lib_search_pathlist.push_back("/usr/lib");
  lib_search_pathlist.push_back("/usr/lib/p7zip");
  lib_search_pathlist.push_back("/usr/local/lib/p7zip");
  lib_search_pathlist.push_back(".");

  for(std::vector<const char *>::iterator lib_search_pathlistIt = lib_search_pathlist.begin(); 
      lib_search_pathlistIt != lib_search_pathlist.end(); 
      lib_search_pathlistIt++)
  {
    string path_prefix = *lib_search_pathlistIt;
    path_prefix += "/";
    path_prefix += tmpName;

    pHandler = dlopen(path_prefix.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if ( pHandler != NULL)
      break;
  }
  return pHandler;
}

void Free7ZLibrary(HMODULE pModule)
{
  dlclose(pModule);
}

#endif
