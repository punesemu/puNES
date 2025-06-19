/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef OS_WINDOWS_H_
#define OS_WINDOWS_H_

#include <windowsx.h>
#include <shlobj.h>
#include "VersionHelpers.h"

static double high_resolution_ms(void);

INLINE void gui_init_os(void) {
	if (IsWindows10OrGreater()) {
		gui.version_os = WIN_TEN;
	} else if (IsWindows8Point1OrGreater()) {
		gui.version_os = WIN_EIGHTP1;
	} else if (IsWindows8OrGreater()) {
		gui.version_os = WIN_EIGHT;
	} else if (IsWindows7SP1OrGreater() || IsWindows7OrGreater()) {
		gui.version_os = WIN_SEVEN;
	} else if (IsWindowsVistaSP2OrGreater() || IsWindowsVistaSP1OrGreater() || IsWindowsVistaOrGreater()) {
		gui.version_os = WIN_VISTA;
	} else if (IsWindowsXPSP3OrGreater() || IsWindowsXPSP2OrGreater() || IsWindowsXPSP1OrGreater() || IsWindowsXPOrGreater()) {
		gui.version_os = WIN_XP;
	}

	/*
	// cerco il numero dei cores
	{
		SYSTEM_INFO info;

		GetSystemInfo(&info);
		gui.cpu_cores = info.dwNumberOfProcessors;
	}
	*/

	// avvio il contatore dei millisecondi
	{
		uint64_t pf;

		QueryPerformanceFrequency((LARGE_INTEGER *)&pf);
		gui.frequency = (double)pf;
		QueryPerformanceCounter((LARGE_INTEGER *)&pf);
		gui.counter_start = pf;
		gui_get_ms = high_resolution_ms;
	}
}
INLINE uTCHAR *gui_home(void) {
	static uTCHAR home[MAX_PATH] = { '\0' };

	if (!ustrlen(home)) {
		switch (gui.version_os) {
			case WIN_EIGHTP1:
			case WIN_EIGHT:
			case WIN_SEVEN:
			case WIN_VISTA:
				// FIXME : non funziona sotto wingw
				//SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, nullptr, &gui.home);
				//break;
			case WIN_XP:
			default:
				SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, (LPWSTR)home);
				break;
		}
	}
	return (&home[0]);
}

void gui_sleep(double ms) {
	if (ms >= 0) {
		Sleep(ms);
	}
}
HWND gui_screen_id(void) {
#if defined (WITH_OPENGL)
	HWND wid = (HWND)qt.screen->wogl->winId();
#elif defined (WITH_D3D9)
	HWND wid = (HWND)qt.screen->wd3d9->winId();
#endif

	return (wid);
}
HWND gui_win_id(void) {
	HWND wid = (HWND)qt.mwin->winId();

	return (wid);
}

char *gui_dup_wchar_to_utf8(uTCHAR *w) {
	int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	char *s = NULL;

	if ((s = (char *)malloc(len))) {
		WideCharToMultiByte(CP_UTF8, 0, w, -1, s, len, 0, 0);
	}

	return (s);
}
unsigned int gui_hardware_concurrency(void) {
	SYSTEM_INFO sysinfo;

	GetSystemInfo(&sysinfo);
	return (sysinfo.dwNumberOfProcessors);
}

static double high_resolution_ms(void) {
	uint64_t time, diff;

	QueryPerformanceCounter((LARGE_INTEGER *)&time);
	diff = ((time - gui.counter_start) * 1000) / gui.frequency;

	return ((double)(diff & 0xffffffff));
}

#endif /* OS_WINDOWS_H_ */
