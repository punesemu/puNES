#include "StdAfx.h"

#include <time.h>

DWORD WINAPI GetTickCount(VOID) {
  return (DWORD)time(0); // FIXME : but only for the seed of the random generator
}

