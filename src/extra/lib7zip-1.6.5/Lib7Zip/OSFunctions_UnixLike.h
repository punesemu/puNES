#ifndef __OS_FUNCTIONS_UNIX_LIKE_H__
#define __OS_FUNCTIONS_UNIX_LIKE_H__

#define GetProcAddress dlsym
#define HMODULE void *

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "dlfcn.h"

#endif // __OS_FUNCTIONS_UNIX_LIKE_H__
