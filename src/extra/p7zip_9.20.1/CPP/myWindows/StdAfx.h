// stdafx.h

#ifndef __STDAFX_H
#define __STDAFX_H


#include "config.h"


#define NO_INLINE /* FIXME */

#ifdef ENV_HAVE_PTHREAD
#include <pthread.h>
#endif

#include "Common/MyWindows.h"
#include "Common/Types.h"

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tchar.h>
#include <wchar.h>
#include <stddef.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#ifdef __NETWARE__
#include <sys/types.h>
#endif

#undef CS /* fix for Solaris 10 x86 */


/***************************/

#ifndef ENV_HAVE_WCHAR__H

EXTERN_C_BEGIN

size_t	wcslen(const wchar_t *);
wchar_t *wcscpy(wchar_t * , const wchar_t * );
wchar_t *wcscat(wchar_t * , const wchar_t * );

EXTERN_C_END

#endif

/***************************/

#define CLASS_E_CLASSNOTAVAILABLE        ((HRESULT)0x80040111L)

/************************* LastError *************************/
inline DWORD WINAPI GetLastError(void) { return errno; }
inline void WINAPI SetLastError( DWORD err ) { errno = err; }

#define AreFileApisANSI() (1)

void Sleep(unsigned millisleep);

typedef pid_t t_processID;

t_processID GetCurrentProcess(void);

#define  NORMAL_PRIORITY_CLASS (0)
#define  IDLE_PRIORITY_CLASS   (10)
void SetPriorityClass(t_processID , int priority);

#ifdef __cplusplus
class wxWindow;
typedef wxWindow *HWND;

#define MB_ICONERROR (0x00000200) // wxICON_ERROR
#define MB_YESNOCANCEL (0x00000002 | 0x00000008 | 0x00000010) // wxYES | wxNO | wxCANCEL
#define MB_ICONQUESTION (0x00000400) // wxICON_QUESTION
#define MB_TASKMODAL  (0) // FIXME
#define MB_SYSTEMMODAL (0) // FIXME

#define MB_OK (0x00000004) // wxOK
#define MB_ICONSTOP (0x00000200) // wxICON_STOP
#define MB_OKCANCEL (0x00000004 | 0x00000010) // wxOK | wxCANCEL

#define MessageBox MessageBoxW
int MessageBoxW(wxWindow * parent, const TCHAR * mes, const TCHAR * title,int flag);

typedef void *HINSTANCE;

typedef          int   INT_PTR;  // FIXME 64 bits ?
typedef unsigned int  UINT_PTR;  // FIXME 64 bits ?
typedef          long LONG_PTR;  // FIXME 64 bits ?
typedef          long DWORD_PTR; // FIXME 64 bits ?
typedef UINT_PTR WPARAM;

/* WARNING
 LPARAM shall be 'long' because of CListView::SortItems and wxListCtrl::SortItems :
*/
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;

#define CALLBACK /* */

/************ LANG ***********/
typedef WORD            LANGID;

LANGID GetUserDefaultLangID(void);
LANGID GetSystemDefaultLangID(void);

#define PRIMARYLANGID(l)        ((WORD)(l) & 0x3ff)
#define SUBLANGID(l)            ((WORD)(l) >> 10)

#if defined( __x86_64__ )

#define _WIN64 1

#endif

#endif

#endif 

