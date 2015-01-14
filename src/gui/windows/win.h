/*
 * win.h
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#ifndef WIN_H_
#define WIN_H_

#if !defined (_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif

#if defined (__cplusplus)
#undef NULL
#endif
#define INITGUID
#include <windows.h>
#undef INITGUID

#define	WIN_EIGHTP1 63
#define	WIN_EIGHT   62
#define	WIN_SEVEN   61
#define	WIN_VISTA   60
#define	WIN_XP64    52
#define	WIN_XP      51

#define exit_thread(value) return

#endif /* WIN_H_ */
