#ifndef __OS_FUNCTIONS_OS2_H__
#define __OS_FUNCTIONS_OS2_H__

#include "unistd.h"
#include "dirent.h"
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#include <os2.h>
#include <sstream>

typedef basic_ostringstream<wchar_t> 	wostringstream;
typedef basic_ostringstream<char> 	ostringstream;

void *GetProcAddress (HMODULE hmod, const char *symbol);

#endif // __OS_FUNCTIONS_OS2_H__
