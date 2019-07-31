/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef UNICODE_DEF_H_
#define UNICODE_DEF_H_

// windows
#if defined (_WIN32)
#include <wchar.h>

typedef wchar_t uTCHAR;

#if defined (_WIN64) || defined (__MINGW64__)
#define ustructstat _stat64i32
#else
#define ustructstat _stat32
#endif

#define ustring wstring

#define uPERCENTs "%ls"
#define uL(string) L##string
#define uPTCHAR(string) (wchar_t *)string

#define usizeof(string) LENGTH(string)
#define uQString QString::fromWCharArray
#define uQStringCD(string) uPTCHAR(string.constData())

#define uvsnprintf vswprintf
#define umemset wmemset
#define ustrncpy wcsncpy
#define usnprintf swprintf
#define uprintf wprintf
#define ustrlen wcslen
#define uaccess _waccess
#define ustat _wstat
#define ufprintf fwprintf
#define ufopen _wfopen
#define ufdopen _wfdopen
#define ustrrchr wcsrchr
#define ustrcasecmp gui_utf_strcasecmp
#define ustrcmp wcscmp
#define ustrncmp wcsncmp
#define ustrcat wcscat
#define uremove _wremove
#define uioctl
#define uopen _wopen
#define uchdir _wchdir
#define ugetcwd _wgetcwd
#define umemcpy wmemcpy
#define ustrchr wcschr
#define ustrdup _wcsdup

// linux
#else

typedef char uTCHAR;

#define ustructstat stat

#define ustring string

#define uPERCENTs "%s"
#define uL(string) string
#define uPTCHAR(string) (char *)string

#define usizeof(string) sizeof(string)
#define uQString QString::fromUtf8
#define uQStringCD(string) uPTCHAR(string.toUtf8().constData())

#define uvsnprintf vsnprintf
#define umemset memset
#define ustrncpy strncpy
#define usnprintf snprintf
#define uprintf printf
#define ustrlen strlen
#define uaccess access
#define ustat stat
#define ufprintf fprintf
#define ufopen fopen
#define ufdopen fdopen
#define ustrrchr strrchr
#define ustrcasecmp strcasecmp
#define ustrcmp strcmp
#define ustrncmp strncmp
#define ustrcat strcat
#define uremove remove
#define uioctl ioctl
#define uopen open
#define uchdir chdir
#define ugetcwd getcwd
#define umemcpy memcpy
#define ustrchr strchr
#define ustrdup strdup

#endif

#endif /* UNICODE_DEF_H_ */
