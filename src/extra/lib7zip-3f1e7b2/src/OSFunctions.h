#ifndef __OS_FUNCTIONS_H__
#define __OS_FUNCTIONS_H__

#if defined(_WIN32)
#include "OSFunctions_Win32.h"
#elif defined(_OS2)
#include "OSFunctions_OS2.h"
#else
#include "OSFunctions_UnixLike.h"
#endif

wstring GetHandlerPath(void * pHandle);
HMODULE Load7ZLibrary(const wstring & library);
void Free7ZLibrary(HMODULE pModule);

#endif //__OS_FUNCTIONS_H__
