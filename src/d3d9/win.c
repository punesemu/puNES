/*
 * win.c
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#if defined MINGW32
#define _WIN32_IE 0x0300
#elif defined MINGW64
#define _WIN32_IE 0x0501
#endif
#include "win.h"
#include <windowsx.h>
#include <shlobj.h>
#include <libgen.h>
#include "version.h"
#include "gfx.h"

#define TOOLBAR_HEIGHT   26
#define FRAME_TL_HEIGHT  (TOOLBAR_HEIGHT - 2)
#define FRAME_TL_WIDTH   SCR_ROWS
#define FRAME_SS_HEIGHT  TOOLBAR_HEIGHT - 2
#define FRAME_SS_WIDTH   1 + BUTTON_SS_WIDTH + 0 + COMBO_SS_WIDTH + 2 + BUTTON_SS_WIDTH + 1
#define BUTTON_SS_HEIGHT FRAME_SS_HEIGHT - 1
#define BUTTON_SS_WIDTH  31
#define COMBO_SS_WIDTH   60
#define SEPARATOR_WIDTH  3

long __stdcall main_proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
double high_resolution_ms(void);

static HWND main_win, d3d_frame;

void gui_init(int argc, char **argv) {
	gui.start = FALSE;

	{
		OSVERSIONINFO win_info;

		info.gui = TRUE;
		ZeroMemory(&win_info, sizeof(OSVERSIONINFO));
		win_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&win_info);
		gui.version_os = ((win_info.dwMajorVersion * 10) | win_info.dwMinorVersion);
	}

	/* cerco la Documents e imposto la directory base */
	{
		switch (gui.version_os) {
			case WIN_EIGHT:
			case WIN_SEVEN:
			case WIN_VISTA:
				// FIXME : non compila sotto mingw
				//hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL,
				//		&gui.home);
				//break;
			case WIN_XP64:
			case WIN_XP:
			default:
				SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, gui.home);
				break;
		}

		if (info.portable) {
			char path[sizeof(info.base_folder)], *dname;
			DWORD length = GetModuleFileName(NULL, (LPSTR) &path, sizeof(path));

			if (length == 0) {
				fprintf(stderr, "INFO: Error resolving exe path.\n");
				info.portable = FALSE;
			} else if (length == sizeof(info.base_folder)) {
				fprintf(stderr, "INFO: Path too long. Truncated.\n");
				info.portable = FALSE;
			}

			dname = dirname(path);
			strcpy(info.base_folder, dname);
		}

		if (!info.portable) {
			sprintf(info.base_folder, "%s/%s", gui.home, NAME);
		}
	}

	/* cerco il numero dei cores */
	{
		SYSTEM_INFO info;

		GetSystemInfo(&info);
		gui.cpu_cores = info.dwNumberOfProcessors;
	}

	/* avvio il contatore dei millisecondi */
	{
		uint64_t pf;

		/*
		 * se il pc non ha il supporto per l'high-resolution
		 * counter allora utilizzo il contatore dell'sdl.
		 */
		if (!QueryPerformanceFrequency((LARGE_INTEGER *) &pf)) {
			//gui_get_mss = sdl_get_ms;
		} else {
			gui.frequency = (double) pf;
			QueryPerformanceCounter((LARGE_INTEGER *) &pf);
			gui.counter_start = pf;
			gui_get_ms = high_resolution_ms;
		}
	}
}
void gui_quit(void) {
	DestroyWindow(main_win);
}
BYTE gui_create(void) {
	WNDCLASSEX wc;
	//INITCOMMONCONTROLSEX icex;
	const char class_name[] = "FHWindowClass";

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) main_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = gui.main_hinstance;
	wc.hIcon = LoadIcon(gui.main_hinstance, MAKEINTRESOURCE(IDI_MYICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	//wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = class_name;
	wc.hIconSm = LoadIcon(gui.main_hinstance, MAKEINTRESOURCE(IDI_MYICON));

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- main window ---------------------------------- */

	main_win = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES,
			class_name,
			"puNES D3D9 window",
	        WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU |
	        WS_MINIMIZEBOX,
	        CW_USEDEFAULT, CW_USEDEFAULT,
	        CW_USEDEFAULT, CW_USEDEFAULT,
	        (HWND) NULL,
	        (HMENU) NULL,
	        gui.main_hinstance,
	        NULL);

	if (main_win == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- D3D frame ---------------------------------- */

	/* creo la finestra a cui attacchero' lo screen D3D */
	//gui.d3d_frame = CreateWindowEx(0, class_name, "puNES D3D frame", WS_CHILD, CW_USEDEFAULT,
	//        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, main_win, (HMENU) NULL, gui.main_hinstance,
	//        NULL );

	d3d_frame = CreateWindowEx(0,
			class_name,
			"puNES D3D frame",
			WS_CHILD,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			main_win,
			(HMENU) NULL,
			gui.main_hinstance,
			NULL);

	if (d3d_frame == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void gui_set_video_mode(void) {
	RECT rc_client, rc_wind;
	POINT pt_diff;

	/* aggiorno la dimensione della finestra principale */
	GetClientRect(main_win, &rc_client);
	GetWindowRect(main_win, &rc_wind);
	pt_diff.x = (rc_wind.right - rc_wind.left) - rc_client.right;
	pt_diff.y = (rc_wind.bottom - rc_wind.top) - rc_client.bottom;
	MoveWindow(main_win, rc_wind.left, rc_wind.top, gfx.w[VIDEO_MODE] + pt_diff.x,
	        gfx.h[VIDEO_MODE] + pt_diff.y + TOOLBAR_HEIGHT, TRUE);
	/* aggiorno la finestra dell'sdl */
	MoveWindow(d3d_frame, 0, 0, gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE], TRUE);
	/* aggiorno la toolbar */
	//MoveWindow(toolbox_frame, 0, gfx.h[VIDEO_MODE], gfx.w[VIDEO_MODE], TOOLBAR_HEIGHT, TRUE);
	/* aggiorno il frame della timeline */
	/*
	{
		WORD rows = FRAME_TL_WIDTH;

		if (cfg->scale == X1) {
			hide_tool_widget();
			if (overscan.enabled) {
				rows = gfx.rows;
			}
		} else {
			show_tool_widget();
		}
		pt_diff.x = gfx.w[VIDEO_MODE] - rows;
		MoveWindow(hFrameTl, pt_diff.x, 0, rows, FRAME_TL_HEIGHT, TRUE);
		MoveWindow(hTimeline, 0, 0, rows - 4, FRAME_TL_HEIGHT, TRUE);
	}
	*/
	//pt_diff.x -= SEPARATOR_WIDTH;
	//MoveWindow(hSepTl, pt_diff.x, 0, SEPARATOR_WIDTH, FRAME_TL_HEIGHT, TRUE);
	/* aggiorno il frame dello saveslot */
	//pt_diff.x -= FRAME_SS_WIDTH;
	//MoveWindow(hFrameSs, pt_diff.x, 0, FRAME_SS_WIDTH, FRAME_SS_HEIGHT, TRUE);
	//pt_diff.x -= SEPARATOR_WIDTH;
	//MoveWindow(hSepSs, pt_diff.x, 0, SEPARATOR_WIDTH, FRAME_TL_HEIGHT, TRUE);
	/* frame vuoto */
	//pt_diff.y = gfx.w[VIDEO_MODE] - pt_diff.x;
	//MoveWindow(hFrameBl, 0, 0, gfx.w[VIDEO_MODE] - pt_diff.y, FRAME_TL_HEIGHT, TRUE);
}
void gui_start(void) {
	/* visualizzo il frame principale */
	ShowWindow(main_win, SW_NORMAL);
	UpdateWindow(main_win);

	/* visualizzo il frame D3D */
	ShowWindow(d3d_frame, SW_SHOWNOACTIVATE);
	UpdateWindow(d3d_frame);

	gui.start = TRUE;

	emu_loop();

	return;
}
void gui_event(void) {
	//BYTE no_process = FALSE;
	MSG msg = { 0 };

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		/*
		 * e' a zero solo quando non ci sono messaggi
		 * e info.pause e' settato.
		 */
		if (!msg.hwnd) {
			Sleep(3);
			continue;
		}
		//if (!TranslateAccelerator(main_win, hAccKeys, &Msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		//}
		msg.hwnd = 0;
	}

	return;
}
HWND gui_window_id(void) {
	return (d3d_frame);
}
void gui_update(void) {
	return;
}
void gui_fullscreen(void) {
	return;
}
void gui_timeline(void) {
	return;
}
void gui_save_slot(BYTE slot) {
	return;
}
int gui_sleep(double ms) {
	return (EXIT_OK);
}
void gui_set_thread_affinity(uint8_t core) {
	return;
}

long __stdcall main_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			info.stop = TRUE;
			break;
		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS);
			return (1);
		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
double high_resolution_ms(void) {
	uint64_t time, diff;

	QueryPerformanceCounter((LARGE_INTEGER *) &time);
	diff = ((time - gui.counter_start) * 1000) / gui.frequency;

	return ((double) (diff & 0xffffffff));
}
