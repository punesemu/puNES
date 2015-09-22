// Windows/DLL.h

#ifndef __WINDOWS_DLL_H
#define __WINDOWS_DLL_H

#include "../Common/MyString.h"

typedef void * HMODULE;

typedef int (*FARPROC)();

namespace NWindows {
namespace NDLL {

class CLibrary
{
  bool LoadOperations(HMODULE newModule);
  HMODULE _module;
public:
  operator HMODULE() const { return _module; }
  HMODULE* operator&() { return &_module; }


  CLibrary():_module(NULL) {};
  ~CLibrary();

  bool Free();

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


  bool Load(LPCTSTR fileName);
  FARPROC GetProc(LPCSTR procName) const;
};

}}

#endif
