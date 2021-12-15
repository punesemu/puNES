// Windows/DLL.h

#ifndef __WINDOWS_DLL_H
#define __WINDOWS_DLL_H

#include "../Common/MyString.h"

typedef void * HMODULE;
// #define LOAD_LIBRARY_AS_DATAFILE 0
typedef int (*FARPROC)();

namespace NWindows {
namespace NDLL {

class CLibrary
{
  HMODULE _module;
public:
  CLibrary(): _module(NULL) {};
  ~CLibrary() { Free(); }

  operator HMODULE() const { return _module; }
  HMODULE* operator&() { return &_module; }
  bool IsLoaded() const { return (_module != NULL); };

  void Attach(HMODULE m)
  {
    Free();
    _module = m;
  }
  HMODULE Detach()
  {
    HMODULE m = _module;
    _module = NULL;
    return m;
  }

  bool Free();
  // bool LoadEx(CFSTR path, DWORD flags = LOAD_LIBRARY_AS_DATAFILE);
  bool Load(CFSTR path);
  FARPROC GetProc(LPCSTR procName) const; //  { return My_GetProcAddress(_module, procName); }
};

bool MyGetModuleFileName(FString &path);

FString GetModuleDirPrefix();

}}

#endif
