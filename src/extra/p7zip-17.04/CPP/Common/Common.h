// Common.h

#ifndef __COMMON_COMMON_H
#define __COMMON_COMMON_H

#include "../../C/Compiler.h"

#include "MyWindows.h"
#include "NewHandler.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[1]))

#define MY_ARRAY_NEW(p, T, size) p = new T[size];

#endif
