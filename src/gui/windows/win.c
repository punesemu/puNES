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
#define _INPUTINLINE_
#include "input.h"
#undef  _INPUTINLINE_
#include "overscan.h"
#include "sdlgfx.h"
#include "sdlsnd.h"
#include "clock.h"
#include "cfgfile.h"
#include "timeline.h"
#include "savestate.h"
#include "version.h"
#include "fps.h"
#include "tas.h"
#include "sdltext.h"
#include "param.h"
#include "ppu.h"
#include "fds.h"
#include "gamegenie.h"
#include "audio_filter.h"
#ifdef OPENGL
#include "opengl.h"
#include "openGL/no_effect.h"
#include "openGL/cube3d.h"
#endif

#define timer_redraw_start()\
	SetTimer(hwnd, IDT_TIMER1, 650, (TIMERPROC) time_handler_redraw)
#define timer_redraw_stop()\
	KillTimer(hwnd, IDT_TIMER1)
#define tlDown(type)\
	emuPause(TRUE);\
	type = TRUE;\
	if (tl.snapsFill) {\
		/* faccio lo screenshot dello screen attuale */\
		memcpy(tl.snaps[TLSNAPFREE] + tl.preview, screen.data, screenSize());\
	}
#define tlUp(type)\
	if (tl.snapsFill) {\
		BYTE snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);\
		if (snap != (tl.snapsFill - 1)) {\
			timelineBack(TLNORMAL, snap);\
		}\
	}\
	SetFocus(hSDL);\
	type = FALSE;\
	emuPause(FALSE)
#define hideToolWidget()\
	ShowWindow(hFrameSs, SW_HIDE)
#define showToolWidget()\
	ShowWindow(hFrameSs, SW_SHOW)

enum { INC, DEC };
enum { SAVE, LOAD };
enum { CHECK, ENAB };

#define TOOLBARHEIGHT  26
#define FRAMETLHEIGHT  (TOOLBARHEIGHT - 2)
#define FRAMETLWIDTH   SCRROWS
#define FRAMESSHEIGHT  TOOLBARHEIGHT - 2
#define FRAMESSWIDTH   1 + BUTTONSSWIDTH + 0 + COMBOSSWIDTH + 2 + BUTTONSSWIDTH + 1
#define BUTTONSSHEIGHT FRAMESSHEIGHT - 1
#define BUTTONSSWIDTH  31
#define COMBOSSWIDTH   60
#define SEPARATORWIDTH 3

long __stdcall mainWinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
long __stdcall timelineProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall saveslotProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall aboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
double highResolutionMs(void);
void open_event(void);
void change_menuitem(BYTE checkORenab, UINT type, UINT bMenuItemID);
void make_reset(BYTE type);
void set_mode(BYTE newmode);
void set_scale(BYTE newscale);
void set_overscan(BYTE newoscan);
void set_filter(BYTE newfilter);
#ifdef OPENGL
void set_rendering(BYTE bool);
void set_vsync(BYTE bool);
#endif
void set_samplerate(int newsamplerate);
void set_channels(int newchannels);
void set_audio_filter(int newfilter);
void set_fps(int newfps);
void set_frame_skip(int newframeskip);
void set_gamegenie(void);
void __stdcall time_handler_redraw(void);
HBITMAP createBitmapMask(HBITMAP hbmColour, COLORREF crTransparent);
void wrapTlPreview(BYTE snap);
void saveslot_incdec(BYTE mode);
void saveslot_action(BYTE mode);
void saveslot_set(BYTE selection);
void fds_eject_insert_disk(void);
void fds_select_side(int side);

static HWND hMainWin, hSDL, hTool;
static HWND hFrameTl, hTimeline;
static HWND hSepTl;
static HWND hFrameSs, hSaveslot, hSaveButton, hLoadButton;
static HWND hSepSs;
static HWND hFrameBl;
HMENU hMainMenu;
HACCEL hAccKeys;
HBITMAP aboutImg, aboutMask;
MONITORINFO mi = { sizeof(mi) };
WINDOWPLACEMENT wpPrev = { sizeof(wpPrev) };

void guiInit(int argc, char **argv) {
	gui.start = FALSE;

	{
		OSVERSIONINFO winInfo;

		info.gui = TRUE;
		ZeroMemory(&winInfo, sizeof(OSVERSIONINFO));
		winInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&winInfo);
		gui.versionOS = ((winInfo.dwMajorVersion * 10) | winInfo.dwMinorVersion);
	}

	/* cerco la Documents e imposto la directory base */
	{
		switch (gui.versionOS) {
			case SEVEN:
			case VISTA:
				// FIXME : non compila sotto mingw
				//hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL,
				//		&gui.home);
				//break;
			case WINXP:
			case WINXP64:
				SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, gui.home);
				break;
		}

		if (info.portable) {
			char path[sizeof(info.baseFolder)], *dname;
			DWORD length = GetModuleFileName(NULL, (LPSTR) &path, sizeof(path));

			if (length == 0) {
				fprintf(stderr, "INFO: Error resolving exe path.\n");
				info.portable = FALSE;
			} else if (length == sizeof(info.baseFolder)) {
				fprintf(stderr, "INFO: Path too long. Truncated.\n");
				info.portable = FALSE;
			}

			dname = dirname(path);
			strcpy(info.baseFolder, dname);
		}

		if (!info.portable) {
			sprintf(info.baseFolder, "%s/%s", gui.home, NAME);
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
			guiGetMs = sdlGetMs;
		} else {
			gui.frequency = (double) pf;
			QueryPerformanceCounter((LARGE_INTEGER *) &pf);
			gui.counterStart = pf;
			guiGetMs = highResolutionMs;
		}
	}
}
BYTE guiCreate(void) {
	WNDCLASSEX wc;
	INITCOMMONCONTROLSEX icex;
	const char className[] = "FHWindowClass";

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) mainWinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = gui.mainhInstance;
	wc.hIcon = LoadIcon(gui.mainhInstance, MAKEINTRESOURCE(IDI_MYICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = className;
	wc.hIconSm = LoadIcon(gui.mainhInstance, MAKEINTRESOURCE(IDI_MYICON));

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* creo la finestra principale */
	hMainWin = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			className,
			"puNES MainFrame",
			WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU
			| WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			(HWND) NULL, (HMENU) NULL, gui.mainhInstance, NULL);

	if (hMainWin == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	hMainMenu = GetMenu(hMainWin);

	hAccKeys = LoadAccelerators(gui.mainhInstance, "IDR_ACCKEYS");

	if (hAccKeys == NULL) {
		MessageBox(NULL, "Window Accelerators Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- SDL window ---------------------------------- */
	/* creo la finestra a cui attacchero' lo screen sdl */
	hSDL = CreateWindowEx(0, className, "puNES SDL Frame", WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT,
	        CW_USEDEFAULT, CW_USEDEFAULT, hMainWin, (HMENU) NULL, gui.mainhInstance, NULL);

	if (hSDL == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- Toolbar ---------------------------------- */

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE,
			CW_USEDEFAULT, CW_USEDEFAULT, 0, TOOLBARHEIGHT, hMainWin, (HMENU) NULL,
			gui.mainhInstance, NULL);

	if (hTool == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- Timeline ---------------------------------- */

	/* Frame Timeline */
	hFrameTl = CreateWindowEx(0, "static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, 0, 0,
			FRAMETLWIDTH, FRAMETLHEIGHT, hTool, NULL, NULL, NULL);

	if (hFrameTl == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SetWindowLongPtr(hFrameTl, GWLP_WNDPROC, (LONG_PTR) timelineProc);

	hTimeline = CreateWindowEx(0, TRACKBAR_CLASS, "Timeline",
			WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_FIXEDLENGTH | TBS_TOOLTIPS, 2, 0,
			FRAMETLWIDTH - 4, FRAMETLHEIGHT, hFrameTl, (HMENU) NULL, gui.mainhInstance, NULL);

	if (hTimeline == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hTimeline, TBM_SETRANGE, TRUE, MAKELONG(0, (TLSNAPS - 1)));
	SendMessage(hTimeline, TBM_SETTHUMBLENGTH, 15, 0);
	SendMessage(hTimeline, TBM_SETPAGESIZE, 0, 0);
	SendMessage(hTimeline, TBM_SETTIPSIDE, TBTS_BOTTOM, 0);

	/* -------------------------------- Separatore Tl -------------------------------- */

	hSepTl = CreateWindowEx(0, "static", "", WS_CHILD | WS_VISIBLE, 0, 0, SEPARATORWIDTH,
			TOOLBARHEIGHT, hTool, (HMENU) NULL, gui.mainhInstance, NULL);

	/* ---------------------------------- Save slot ---------------------------------- */

	HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

	/* Frame Saveslot */
	hFrameSs = CreateWindowEx(WS_EX_WINDOWEDGE, "static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
			0, 0, FRAMESSWIDTH,
			FRAMESSHEIGHT, hTool, NULL, NULL, NULL);

	if (hFrameSs == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SetWindowLongPtr(hFrameSs, GWLP_WNDPROC, (LONG_PTR) saveslotProc);

	hSaveslot = CreateWindowEx(0, "COMBOBOX", "Saveslot",
			WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_OWNERDRAWFIXED,
			1 + BUTTONSSWIDTH + 0, 1, COMBOSSWIDTH, 130, hFrameSs, (HMENU) ID_SAVESLOT_CB,
			gui.mainhInstance, NULL);

	if (hSaveslot == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hSaveslot, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

	hSaveButton = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 1, 0,
			BUTTONSSWIDTH,
			BUTTONSSHEIGHT, hFrameSs, (HMENU) ID_SAVESLOT_BS, gui.mainhInstance, NULL);

	if (hSaveButton == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hSaveButton, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

	hLoadButton = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			1 + BUTTONSSWIDTH + 0 + COMBOSSWIDTH + 2, 0, BUTTONSSWIDTH,
			BUTTONSSHEIGHT, hFrameSs, (HMENU) ID_SAVESLOT_BL, gui.mainhInstance, NULL);

	if (hLoadButton == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hLoadButton, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

	{
		BYTE i;

		for (i = 0; i < SSAVAILABLE; i++) {
			char item[10];
			sprintf(item, "Slot %d", i);
			SendMessage(hSaveslot, CB_ADDSTRING, 0, (LPARAM) item);
		}
	}

	SendMessage(hSaveslot, CB_SETCURSEL, 0, 0);

	/* -------------------------------- Separatore Ss -------------------------------- */

	hSepSs = CreateWindowEx(0, "static", "", WS_CHILD | WS_VISIBLE, 0, 0, SEPARATORWIDTH,
			TOOLBARHEIGHT, hTool, (HMENU) NULL, gui.mainhInstance, NULL);

	/* -------------------------------- Frame vuoto -------------------------------- */

	/* Frame Saveslot */
	hFrameBl = CreateWindowEx(WS_EX_WINDOWEDGE, "static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
			0, 0, 0,
			FRAMESSHEIGHT, hTool, NULL, NULL, NULL);

	if (hFrameBl == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void guiSetVideoMode(void) {
	RECT rcClientMainWin, rcWindMainWin;
	POINT ptDiff;

	/* aggiorno la dimensione della finestra principale */
	GetClientRect(hMainWin, &rcClientMainWin);
	GetWindowRect(hMainWin, &rcWindMainWin);
	ptDiff.x = (rcWindMainWin.right - rcWindMainWin.left) - rcClientMainWin.right;
	ptDiff.y = (rcWindMainWin.bottom - rcWindMainWin.top) - rcClientMainWin.bottom;
	MoveWindow(hMainWin, rcWindMainWin.left, rcWindMainWin.top, gfx.w[VIDEOMODE] + ptDiff.x,
			gfx.h[VIDEOMODE] + ptDiff.y + TOOLBARHEIGHT, TRUE);
	/* aggiorno la finestra dell'sdl */
	MoveWindow(hSDL, 0, 0, gfx.w[VIDEOMODE], gfx.h[VIDEOMODE], TRUE);
	/* aggiorno la toolbar */
	MoveWindow(hTool, 0, gfx.h[VIDEOMODE], gfx.w[VIDEOMODE], TOOLBARHEIGHT, TRUE);
	/* aggiorno il frame della timeline */
	{
		WORD rows = FRAMETLWIDTH;

		if (gfx.scale == X1) {
			hideToolWidget();
			if (overscan.enabled) {
				rows = gfx.rows;
			}
		} else {
			showToolWidget();
		}
		ptDiff.x = gfx.w[VIDEOMODE] - rows;
		MoveWindow(hFrameTl, ptDiff.x, 0, rows, FRAMETLHEIGHT, TRUE);
		MoveWindow(hTimeline, 0, 0, rows - 4, FRAMETLHEIGHT, TRUE);
	}
	ptDiff.x -= SEPARATORWIDTH;
	MoveWindow(hSepTl, ptDiff.x, 0, SEPARATORWIDTH, FRAMETLHEIGHT, TRUE);
	/* aggiorno il frame dello saveslot */
	ptDiff.x -= FRAMESSWIDTH;
	MoveWindow(hFrameSs, ptDiff.x, 0, FRAMESSWIDTH, FRAMESSHEIGHT, TRUE);
	ptDiff.x -= SEPARATORWIDTH;
	MoveWindow(hSepSs, ptDiff.x, 0, SEPARATORWIDTH, FRAMETLHEIGHT, TRUE);
	/* frame vuoto */
	ptDiff.y = gfx.w[VIDEOMODE] - ptDiff.x;
	MoveWindow(hFrameBl, 0, 0, gfx.w[VIDEOMODE] - ptDiff.y, FRAMETLHEIGHT, TRUE);
}
void guiStart(void) {
	/* visualizzo il frame principale */
	ShowWindow(hMainWin, SW_NORMAL);
	UpdateWindow(hMainWin);
	/* visualizzo il frame sdl */
	ShowWindow(hSDL, SW_SHOWNOACTIVATE);
	UpdateWindow(hSDL);
	/* visualizzo la toolbar */
	ShowWindow(hTool, SW_SHOWNOACTIVATE);
	UpdateWindow(hTool);
	/* visualizzo il frame della timeline */
	ShowWindow(hFrameTl, SW_SHOWNOACTIVATE);
	UpdateWindow(hFrameTl);
	/* visualizzo la timeline */
	ShowWindow(hTimeline, SW_SHOWNOACTIVATE);
	UpdateWindow(hTimeline);

	/* setto il focus sulla finestra sdl */
	SetForegroundWindow(hSDL);
	SetFocus(hSDL);

	gui.start = TRUE;

	emuLoop();
	return;
}
void guiEvent(void) {
	BYTE noProcess = FALSE;
	MSG Msg = { 0 };

	/* SDL */
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
		/*
		 * e' a zero solo quando non ci sono messaggi
		 * e info.pause e' settato.
		 */
		if (!Msg.hwnd) {
			Sleep(3);
			continue;
		}
		//fprintf(stderr, "0: %X %X\n", Msg.message, LOWORD(Msg.wParam));
		switch (Msg.message) {
			case WM_KEYDOWN: {
				switch(LOWORD(Msg.wParam)) {
					case VK_CONTROL:
						if (!tl.key) {
							tlDown(tl.key);
						}
						noProcess = TRUE;
						break;
					case VK_SHIFT:
						fpsFastForward();
						noProcess = TRUE;
						break;
					case VK_LEFT:
						if (tl.key) {
							BYTE snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);
							if (snap) {wrapTlPreview(snap - 1);}
							noProcess = TRUE;
						}
						break;
					case VK_RIGHT:
						if (tl.key) {
							BYTE snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);
							if (snap < (TLSNAPS - 1)) {wrapTlPreview(snap + 1);}
							noProcess = TRUE;
						}
						break;
#ifdef OPENGL
					case VK_ESCAPE:
						if (gfx.fullscreen == FULLSCR) {
							guiFullscreen();
						}
						noProcess = TRUE;
						break;
#endif
				}
				if (!tas.type && !noProcess) {
					if (inputPort1 && !inputPort1(PRESSED, LOWORD(Msg.wParam), KEYBOARD, &port1)) {
						break;
					}
					if (inputPort2) {
						inputPort2(PRESSED, LOWORD(Msg.wParam), KEYBOARD, &port2);
					}
				}
				break;
			}
			case WM_KEYUP: {
				switch (LOWORD(Msg.wParam)) {
					case VK_CONTROL:
						if (tl.key) {tlUp(tl.key);}
						noProcess = TRUE;
						break;
					case VK_SHIFT:
						fpsNormalize();
						noProcess = TRUE;
						break;
				}
				if (!tas.type && !noProcess) {
					if (inputPort1 && !inputPort1(RELEASED, LOWORD(Msg.wParam), KEYBOARD, &port1)) {
						break;
					}
					if (inputPort2) {
						inputPort2(RELEASED, LOWORD(Msg.wParam), KEYBOARD, &port2);
					}
				}
				break;
			}
			case WM_LBUTTONDOWN:
				gui.left_button = TRUE;
#ifdef OPENGL
				//opengl.xDiff = GET_X_LPARAM(Msg.lParam) - (opengl.yRotate * slowFactor);
				//opengl.yDiff = -GET_Y_LPARAM(Msg.lParam) + (opengl.xRotate * slowFactor);
				opengl.xDiff = gui.x - (opengl.yRotate * slowFactor);
				opengl.yDiff = -gui.y + (opengl.xRotate * slowFactor);
				break;
#endif
			case WM_RBUTTONDOWN:
				gui.right_button = TRUE;
				break;
			case WM_MOUSEMOVE:
				if (Msg.hwnd == hSDL) {
					gui.x = GET_X_LPARAM(Msg.lParam);
					gui.y = GET_Y_LPARAM(Msg.lParam);
				}
#ifdef OPENGL
				if (gui.left_button && opengl.rotation) {
					opengl.xRotate = (gui.y + opengl.yDiff) / slowFactor;
					opengl.yRotate = (gui.x - opengl.xDiff) / slowFactor;
				}
#endif
				break;
			case WM_LBUTTONUP:
				gui.left_button = FALSE;
				break;
			case WM_RBUTTONUP:
				gui.right_button = FALSE;
				break;
		}
		if (!TranslateAccelerator(hMainWin, hAccKeys, &Msg)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		Msg.hwnd = 0;
	}

	if (info.no_rom | info.pause) {
		return;
	}

	if (tas.type) {
		tasFrame();
		return;
	}

	jsControl(&js1, &port1);
	/* i due joystick non possono essere gli stessi */
	if (port2.joyID != port1.joyID) {
		jsControl(&js2, &port2);
	}
	inputTurboButtonsControl(&port1);
	inputTurboButtonsControl(&port2);
	return;
}
HWND guiWindowID(void) {
	return(hSDL);
}
void guiUpdate(void) {
	WORD id = 0;
	char title[255];

	/* aggiorno il titolo */
	emuSetTitle(title);
	SetWindowText(hMainWin, title);

	/* checko le voci di menu corrette */

	/* FDS */
	if (fds.info.enabled) {
		BYTE i;

		HMENU menuNES = GetSubMenu(hMainMenu, 1);
		MENUITEMINFO menuitem;

		if (fds.drive.disk_ejected) {
			menuitem.dwTypeData = "&Insert disk\tALT+E";
		} else {
			menuitem.dwTypeData = "&Eject disk\tALT+E";
		}
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STRING | MIIM_STATE;
		menuitem.fState = MFS_ENABLED;
		SetMenuItemInfo(menuNES, IDM_NES_FDS_EJECT, FALSE, &menuitem);

		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_ENABLED;
		SetMenuItemInfo(menuNES, 3, TRUE, &menuitem);

		for (i = 0; i < (IDM_NES_FDS_DISK_SIDE7 + 1) - IDM_NES_FDS_DISK_SIDE0; i++) {
			if (i < fds.info.total_sides) {
				change_menuitem(ENAB, MF_HILITE, IDM_NES_FDS_DISK_SIDE0 + i);
			} else {
				change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE0 + i);
			}
			if (i == fds.drive.side_inserted) {
				change_menuitem(CHECK, MF_CHECKED, IDM_NES_FDS_DISK_SIDE0 + i);
			} else {
				change_menuitem(CHECK, MF_UNCHECKED, IDM_NES_FDS_DISK_SIDE0 + i);
			}
		}
	} else {
		HMENU menuNES = GetSubMenu(hMainMenu, 1);
		MENUITEMINFO menuitem;

		menuitem.dwTypeData = "Ej&ect/Insert disk\tALT+E";
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STRING | MIIM_STATE;
		menuitem.fState = MFS_DISABLED;
		SetMenuItemInfo(menuNES, IDM_NES_FDS_EJECT, FALSE, &menuitem);

		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_GRAYED;
		SetMenuItemInfo(menuNES, 3, TRUE, &menuitem);

		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE0);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE1);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE2);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE3);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE4);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE5);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE6);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SIDE7);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_DISK_SWITCH);
		change_menuitem(ENAB, MF_GRAYED, IDM_NES_FDS_EJECT);
	}

	/* Save slot */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVE_0);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVE_1);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVE_2);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVE_3);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVE_4);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVE_5);
	switch (savestate.slot) {
		case 0:
			id = IDM_SET_SAVE_0;
			break;
		case 1:
			id = IDM_SET_SAVE_1;
			break;
		case 2:
			id = IDM_SET_SAVE_2;
			break;
		case 3:
			id = IDM_SET_SAVE_3;
			break;
		case 4:
			id = IDM_SET_SAVE_4;
			break;
		case 5:
			id = IDM_SET_SAVE_5;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);
	if (savestate.slotState[savestate.slot]) {
		change_menuitem(ENAB, MF_HILITE, IDM_SET_SAVE_LOAD);
		EnableWindow(hLoadButton, TRUE);
	} else {
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_SAVE_LOAD);
		EnableWindow(hLoadButton, FALSE);
	}
	RedrawWindow(hSaveslot, NULL, NULL, RDW_INVALIDATE);

	/* Mode */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_MODE_PAL);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_MODE_NTSC);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_MODE_DENDY);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_MODE_AUTO);
	if (cfg->mode == AUTO) {
		id = IDM_SET_MODE_AUTO;
	} else if (machine.type == PAL) {
		id = IDM_SET_MODE_PAL;
	} else if (machine.type == NTSC) {
		id = IDM_SET_MODE_NTSC;
	} else if (machine.type == DENDY) {
		id = IDM_SET_MODE_DENDY;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Fps */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_DEFAULT);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_60);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_59);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_58);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_57);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_56);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_55);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_54);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_53);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_52);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_51);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_50);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_49);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_48);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_47);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_46);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_45);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FPS_44);
	switch (cfg->fps) {
		case FPSDEFAULT:
			id = IDM_SET_FPS_DEFAULT;
			break;
		case FPS60:
			id = IDM_SET_FPS_60;
			break;
		case FPS59:
			id = IDM_SET_FPS_59;
			break;
		case FPS58:
			id = IDM_SET_FPS_58;
			break;
		case FPS57:
			id = IDM_SET_FPS_57;
			break;
		case FPS56:
			id = IDM_SET_FPS_56;
			break;
		case FPS55:
			id = IDM_SET_FPS_55;
			break;
		case FPS54:
			id = IDM_SET_FPS_54;
			break;
		case FPS53:
			id = IDM_SET_FPS_53;
			break;
		case FPS52:
			id = IDM_SET_FPS_52;
			break;
		case FPS51:
			id = IDM_SET_FPS_51;
			break;
		case FPS50:
			id = IDM_SET_FPS_50;
			break;
		case FPS49:
			id = IDM_SET_FPS_49;
			break;
		case FPS48:
			id = IDM_SET_FPS_48;
			break;
		case FPS47:
			id = IDM_SET_FPS_47;
			break;
		case FPS46:
			id = IDM_SET_FPS_46;
			break;
		case FPS45:
			id = IDM_SET_FPS_45;
			break;
		case FPS44:
			id = IDM_SET_FPS_44;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Frame skip */
	for (id = IDM_SET_FSK_DEFAULT; id <= IDM_SET_FSK_9; id++) {
		if (cfg->frameskip == (id - IDM_SET_FSK_DEFAULT)) {
			change_menuitem(CHECK, MF_CHECKED, id);
		} else {
			change_menuitem(CHECK, MF_UNCHECKED, id);
		}
	}

	/* Size */
	if (gfx.filter != NOFILTER) {
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_SIZE_1X);
	} else {
		change_menuitem(ENAB, MF_HILITE, IDM_SET_SIZE_1X);
	}
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_1X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_2X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_3X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_4X);
	if (!gfx.fullscreen) {
		switch (gfx.scale) {
			case X1:
				id = IDM_SET_SIZE_1X;
				break;
			case X2:
				id = IDM_SET_SIZE_2X;
				break;
			case X3:
				id = IDM_SET_SIZE_3X;
				break;
			case X4:
				id = IDM_SET_SIZE_4X;
				break;
		}
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Overscan */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_ON);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_OFF);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_DEF);
	switch (gfx.overscan) {
		case OSCANON:
			id = IDM_SET_OSCAN_ON;
			break;
		case OSCANOFF:
			id = IDM_SET_OSCAN_OFF;
			break;
		case OSCANDEF:
			id = IDM_SET_OSCAN_DEF;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Overscan/Default value */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_DEFAULT_ON);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_DEFAULT_OFF);
	switch (gfx.overscanDefault) {
		case OSCANON:
			id = IDM_SET_OSCAN_DEFAULT_ON;
			break;
		case OSCANOFF:
			id = IDM_SET_OSCAN_DEFAULT_OFF;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Filter */
	if (gfx.bitperpixel < 32) {
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_HQ2X);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_HQ3X);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_HQ4X);
	} else {
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_HQ2X);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_HQ3X);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_HQ4X);
	}
#ifdef OPENGL
	{
		HMENU menu_to_change;
		MENUITEMINFO menuitem;

		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;

		/* Video/Rendering */
		menu_to_change = GetSubMenu(GetSubMenu(GetSubMenu(hMainMenu, 2), 2), 0);

		if (opengl.supported) {
			menuitem.fState = MFS_ENABLED;
		} else {
			menuitem.fState = MFS_DISABLED;
		}
		/* Video/Rendering/OpenGL */
		SetMenuItemInfo(menu_to_change, 1, TRUE, &menuitem);

		if (opengl.glsl.compliant) {
			menuitem.fState = MFS_ENABLED;
		} else {
			menuitem.fState = MFS_DISABLED;
		}
		/* Video/Rendering/OpenGL GLSL */
		SetMenuItemInfo(menu_to_change, 2, TRUE, &menuitem);

		menu_to_change = GetSubMenu(GetSubMenu(GetSubMenu(hMainMenu, 2), 2), 7);

		if (opengl.glsl.enabled) {
			change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_POSPHOR);
			change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_SCANLINE);
			change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_DBL);

			menuitem.fState = MFS_ENABLED;

			/* Video/Filter/CRT */
			SetMenuItemInfo(menu_to_change, 5, TRUE, &menuitem);

			change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_CRTCURVE);
			change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_CRTNOCURVE);
		} else {
			change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_POSPHOR);
			change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_SCANLINE);
			change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_DBL);

			menuitem.fState = MFS_DISABLED;

			/* Video/Filter/CRT */
			SetMenuItemInfo(menu_to_change, 5, TRUE, &menuitem);

			change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_CRTCURVE);
			change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_CRTNOCURVE);
		}
	}
#endif
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_NOFILTER);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_BILINEAR);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_SCALE2X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_SCALE3X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_SCALE4X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_HQ2X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_HQ3X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_HQ4X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_RGBNTSCCOM);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_RGBNTSCSVD);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_RGBNTSCRGB);
#ifdef OPENGL
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_POSPHOR);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_SCANLINE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_DBL);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_CRTCURVE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_CRTNOCURVE);
#endif
	switch (gfx.filter) {
		case NOFILTER:
			id = IDM_SET_FILTER_NOFILTER;
			break;
		case BILINEAR:
			id = IDM_SET_FILTER_BILINEAR;
			break;
#ifdef OPENGL
		case POSPHOR:
			id = IDM_SET_FILTER_POSPHOR;
			break;
		case SCANLINE:
			id = IDM_SET_FILTER_SCANLINE;
			break;
		case DBL:
			id = IDM_SET_FILTER_DBL;
			break;
		case CRTCURVE:
			id = IDM_SET_FILTER_CRTCURVE;
			break;
		case CRTNOCURVE:
			id = IDM_SET_FILTER_CRTNOCURVE;
			break;
#endif
		case SCALE2X:
			id = IDM_SET_FILTER_SCALE2X;
			break;
		case SCALE3X:
			id = IDM_SET_FILTER_SCALE3X;
			break;
		case SCALE4X:
			id = IDM_SET_FILTER_SCALE4X;
			break;
		case HQ2X:
			id = IDM_SET_FILTER_HQ2X;
			break;
		case HQ3X:
			id = IDM_SET_FILTER_HQ3X;
			break;
		case HQ4X:
			id = IDM_SET_FILTER_HQ4X;
			break;
		case RGBNTSC:
			switch (gfx.ntscFormat) {
				case COMPOSITE:
					id = IDM_SET_FILTER_RGBNTSCCOM;
					break;
				case SVIDEO:
					id = IDM_SET_FILTER_RGBNTSCSVD;
					break;
				case RGBMODE:
					id = IDM_SET_FILTER_RGBNTSCRGB;
					break;
			}
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Palette */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_PALETTE_PAL);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_PALETTE_NTSC);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_PALETTE_SONY);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_PALETTE_MONO);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_PALETTE_GREEN);
	switch (gfx.palette) {
		case PALETTEPAL:
			id = IDM_SET_PALETTE_PAL;
			break;
		case PALETTENTSC:
			id = IDM_SET_PALETTE_NTSC;
			break;
		case PALETTESONY:
			id = IDM_SET_PALETTE_SONY;
			break;
		case PALETTEMONO:
			id = IDM_SET_PALETTE_MONO;
			break;
		case PALETTEGREEN:
			id = IDM_SET_PALETTE_GREEN;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

#ifdef OPENGL
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_RENDERING_SOFTWARE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_RENDERING_OPENGL);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_RENDERING_GLSL);
	if (gfx.opengl) {
		HMENU menuSettings = GetSubMenu(hMainMenu, 2);
		HMENU menuVideo = GetSubMenu(menuSettings, 2);
		MENUITEMINFO menuitem;

		/* VSync */
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_ENABLED;
		SetMenuItemInfo(menuVideo, 3, TRUE, &menuitem);

		/* questi li abilito solo se non c'e' come input lo zapper */
		if ((port1.type != ZAPPER) && (port2.type != ZAPPER)) {
			menuitem.fState = MFS_ENABLED;
			change_menuitem(ENAB, MF_ENABLED, IDM_SET_EFFECT_CUBE);
		} else {
			menuitem.fState = MFS_GRAYED;
			change_menuitem(ENAB, MF_GRAYED, IDM_SET_EFFECT_CUBE);
		}
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		SetMenuItemInfo(menuVideo, 9, TRUE, &menuitem);

		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FULLSCREEN);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_STRETCHFLSCR);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_VSYNC_ON);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_VSYNC_OFF);

		if (!opengl.glsl.compliant) {
			id = IDM_SET_RENDERING_OPENGL;
		} else if (!opengl.glsl.enabled) {
			id = IDM_SET_RENDERING_OPENGL;
		} else {
			id = IDM_SET_RENDERING_GLSL;
		}
	} else {
		HMENU menuSettings = GetSubMenu(hMainMenu, 2);
		HMENU menuVideo = GetSubMenu(menuSettings, 2);
		MENUITEMINFO menuitem;

		/* VSync */
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_DISABLED;
		SetMenuItemInfo(menuVideo, 3, TRUE, &menuitem);
		/* Effect */
		SetMenuItemInfo(menuVideo, 9, TRUE, &menuitem);

		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FULLSCREEN);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_STRETCHFLSCR);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_VSYNC_ON);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_VSYNC_OFF);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_EFFECT_CUBE);

		id = IDM_SET_RENDERING_SOFTWARE;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_EFFECT_CUBE);
	if (opengl.rotation) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_EFFECT_CUBE);
	}

	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_VSYNC_ON);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_VSYNC_OFF);
	if (gfx.vsync) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_VSYNC_ON);
	} else {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_VSYNC_OFF);
	}

	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_STRETCHFLSCR);
	if (!opengl.aspectRatio && gfx.opengl) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_STRETCHFLSCR);
	}
#endif

	/* Samplerate */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAMPLERATE_44100);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAMPLERATE_22050);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAMPLERATE_11025);
	switch (cfg->samplerate) {
		case S44100:
			id = IDM_SET_SAMPLERATE_44100;
			break;
		case S22050:
			id = IDM_SET_SAMPLERATE_22050;
			break;
		case S11025:
			id = IDM_SET_SAMPLERATE_11025;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Channels */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_CHANNELS_MONO);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_CHANNELS_STEREO);
	switch (cfg->channels) {
		case MONO:
			id = IDM_SET_CHANNELS_MONO;
			break;
		case STEREO:
			id = IDM_SET_CHANNELS_STEREO;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Audio Filter */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_FILTER_NONE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_FILTER_SIMPLE);
	switch (cfg->audio_filter) {
		case AF_NONE:
			id = IDM_SET_AUDIO_FILTER_NONE;
			break;
		case AF_SIMPLE:
			id = IDM_SET_AUDIO_FILTER_SIMPLE;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Audio Enable */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_ENABLE);
	if (cfg->audio) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_AUDIO_ENABLE);
	}

	/* Game Genie */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_GAMEGENIE);
	if (gamegenie.enabled) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_GAMEGENIE);
	}

	/* Save on exit */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVEONEXIT);
	if (cfg->saveOnExit) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_SAVEONEXIT);
	}
}
#ifdef OPENGL
void guiFullscreen(void) {
	emuPause(TRUE);

	/* nascondo la finestra */
	ShowWindow(hMainWin, SW_HIDE);

	if (gfx.fullscreen == NOFULLSCR || gfx.fullscreen == NOCHANGE) {
		/* salvo il valore scale prima del fullscreen */
		gfx.scaleBeforeFullscreen = gfx.scale;
		/* trovo la risoluzione del monitor in uso */
		GetMonitorInfo(MonitorFromWindow(hMainWin, MONITOR_DEFAULTTOPRIMARY), &mi);
		gfx.w[MONITOR] = mi.rcMonitor.right - mi.rcMonitor.left;
		gfx.h[MONITOR] = mi.rcMonitor.bottom - mi.rcMonitor.top;
		/*salvo la posizione della finestra */
		if (gui.start) {
			GetWindowPlacement(hMainWin, &wpPrev);
		} else {
			wpPrev.rcNormalPosition.bottom = 0;
		}
		/* dissocio il menu dalla finestra */
		SetMenu(hMainWin, NULL);
		/* abilito il fullscreen */
		gfxSetScreen(NOCHANGE, NOCHANGE, FULLSCR, NOCHANGE, FALSE);
		/* disabilito la visualizzazione del puntatore */
		if (!opengl.rotation && (port1.type != ZAPPER) && (port2.type != ZAPPER)) {
			SDL_ShowCursor(SDL_DISABLE);
		}
		/* queste sono le cose che devo disabilitare per il fullscreen */
		SetWindowLongPtr(hMainWin, GWL_STYLE,
		        GetWindowLong(hMainWin, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER | WS_SYSMENU));
		SetWindowLongPtr(hMainWin, GWL_EXSTYLE,
		        (GetWindowLongPtr(hMainWin, GWL_EXSTYLE) | WS_EX_APPWINDOW | WS_EX_TOPMOST)
		                & ~WS_EX_CLIENTEDGE);
		/* muovo la finestra al margine superiore destro del monitor */
		MoveWindow(hMainWin, mi.rcMonitor.left, mi.rcMonitor.top, gfx.w[VIDEOMODE],
		        gfx.h[VIDEOMODE], TRUE);
	} else {
		/* ribilito gli stili della finestra corretti */
		SetWindowLongPtr(hMainWin, GWL_STYLE,
			        GetWindowLong(hMainWin, GWL_STYLE) | WS_CAPTION | WS_BORDER | WS_SYSMENU);
		SetWindowLongPtr(hMainWin, GWL_EXSTYLE,
		        (GetWindowLongPtr(hMainWin, GWL_EXSTYLE) | WS_EX_CLIENTEDGE)
		                & ~(WS_EX_APPWINDOW | WS_EX_TOPMOST));
		/* riassocio il menu */
		SetMenu(hMainWin, hMainMenu);
		/* ripristino i valori di scale ed esco dal fullscreen */
		gfxSetScreen(gfx.scaleBeforeFullscreen, NOCHANGE, NOFULLSCR, NOCHANGE, FALSE);
		/* riabilito la visualizzazione del puntatore */
		SDL_ShowCursor(SDL_ENABLE);
		/* posiziono la finestra alle coordinate precedenti il fullscreen */
		if (wpPrev.rcNormalPosition.bottom) {
			SetWindowPlacement(hMainWin, &wpPrev);
		}
	}

	/* visualizzo la finestra */
	ShowWindow(hMainWin, SW_NORMAL);
	/* setto il focus*/
	SetFocus(hSDL);

	emuPause(FALSE);
}
#endif
void guiTimeline(void) {
	SendMessage(hTimeline, TBM_SETPOS, TRUE, tl.snapsFill - 1);
}
void guiSavestate(BYTE slot) {
	if (slot >= SSAVAILABLE) {
		slot = SSAVAILABLE - 1;
	}
	saveslot_set(slot);
}
int guiSleep(double ms) {
	if (ms > 0) {
		Sleep(ms);
	}
	return (EXIT_OK);
}
void guiSetThreadAffinity(uint8_t core) {
	SetThreadAffinityMask(GetCurrentThread(), core + 1);
}

/* funzioni interne */
long __stdcall mainWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_ENTERSIZEMOVE:
		case WM_ENTERMENULOOP:
			emuPause(TRUE);
			//timer_redraw_start();
			break;
		case WM_EXITSIZEMOVE:
		case WM_EXITMENULOOP:
			emuPause(FALSE);
			//timer_redraw_stop();
			SetFocus(hSDL);
			break;
			//case 293:
			//case WM_ENTERIDLE:
			//case WM_MENUSELECT:
			//case WM_INITMENUPOPUP:
			//time_handler_redraw();
			//break;
		case WM_TIMER: {
			switch (wParam) {
				case IDT_TIMER1:
					return (0);
			}
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDM_FILE_OPEN:
					/*
					 * nella finestra di apertura file posso esserci
					 * arrivato anche premendo CTRL+O, quindi in uscita
					 * potrei ancora avere il tl.key settato.
					 */
					tl.key = FALSE;
					open_event();
					SetFocus(hSDL);
					break;
				case IDM_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, EXIT_SUCCESS, 0);
					break;
				case IDM_NES_SOFT:
					make_reset(RESET);
					break;
				case IDM_NES_HARD:
					make_reset(HARD);
					break;
				case IDM_NES_FDS_DISK_SIDE0:
					fds_select_side(0);
					break;
				case IDM_NES_FDS_DISK_SIDE1:
					fds_select_side(1);
					break;
				case IDM_NES_FDS_DISK_SIDE2:
					fds_select_side(2);
					break;
				case IDM_NES_FDS_DISK_SIDE3:
					fds_select_side(3);
					break;
				case IDM_NES_FDS_DISK_SIDE4:
					fds_select_side(4);
					break;
				case IDM_NES_FDS_DISK_SIDE5:
					fds_select_side(5);
					break;
				case IDM_NES_FDS_DISK_SIDE6:
					fds_select_side(6);
					break;
				case IDM_NES_FDS_DISK_SIDE7:
					fds_select_side(7);
					break;
				case IDM_NES_FDS_DISK_SWITCH:
					fds_select_side(0xFFF);
					break;
				case IDM_NES_FDS_EJECT:
					fds_eject_insert_disk();
					break;
				case IDM_SET_SAVE_SAVE:
					saveslot_action(SAVE);
					break;
				case IDM_SET_SAVE_LOAD:
					saveslot_action(LOAD);
					break;
				case IDM_SET_SAVE_INC:
					saveslot_incdec(INC);
					break;
				case IDM_SET_SAVE_DEC:
					saveslot_incdec(DEC);
					break;
				case IDM_SET_SAVE_0:
					saveslot_set(0);
					break;
				case IDM_SET_SAVE_1:
					saveslot_set(1);
					break;
				case IDM_SET_SAVE_2:
					saveslot_set(2);
					break;
				case IDM_SET_SAVE_3:
					saveslot_set(3);
					break;
				case IDM_SET_SAVE_4:
					saveslot_set(4);
					break;
				case IDM_SET_SAVE_5:
					saveslot_set(5);
					break;
				case IDM_SET_MODE_PAL:
					set_mode(PAL);
					break;
				case IDM_SET_MODE_NTSC:
					set_mode(NTSC);
					break;
				case IDM_SET_MODE_DENDY:
					set_mode(DENDY);
					break;
				case IDM_SET_MODE_AUTO:
					set_mode(AUTO);
					break;
				case IDM_SET_FPS_DEFAULT:
					set_fps(FPSDEFAULT);
					break;
				case IDM_SET_FPS_60:
					set_fps(FPS60);
					break;
				case IDM_SET_FPS_59:
					set_fps(FPS59);
					break;
				case IDM_SET_FPS_58:
					set_fps(FPS58);
					break;
				case IDM_SET_FPS_57:
					set_fps(FPS57);
					break;
				case IDM_SET_FPS_56:
					set_fps(FPS56);
					break;
				case IDM_SET_FPS_55:
					set_fps(FPS55);
					break;
				case IDM_SET_FPS_54:
					set_fps(FPS54);
					break;
				case IDM_SET_FPS_53:
					set_fps(FPS53);
					break;
				case IDM_SET_FPS_52:
					set_fps(FPS52);
					break;
				case IDM_SET_FPS_51:
					set_fps(FPS51);
					break;
				case IDM_SET_FPS_50:
					set_fps(FPS50);
					break;
				case IDM_SET_FPS_49:
					set_fps(FPS49);
					break;
				case IDM_SET_FPS_48:
					set_fps(FPS48);
					break;
				case IDM_SET_FPS_47:
					set_fps(FPS47);
					break;
				case IDM_SET_FPS_46:
					set_fps(FPS46);
					break;
				case IDM_SET_FPS_45:
					set_fps(FPS45);
					break;
				case IDM_SET_FPS_44:
					set_fps(FPS44);
					break;
				case IDM_SET_FSK_DEFAULT:
				case IDM_SET_FSK_1:
				case IDM_SET_FSK_2:
				case IDM_SET_FSK_3:
				case IDM_SET_FSK_4:
				case IDM_SET_FSK_5:
				case IDM_SET_FSK_6:
				case IDM_SET_FSK_7:
				case IDM_SET_FSK_8:
				case IDM_SET_FSK_9:
					set_frame_skip(LOWORD(wParam) - IDM_SET_FSK_DEFAULT);
					break;
				case IDM_SET_SIZE_1X:
					/* 1x */
					set_scale(X1);
					break;
				case IDM_SET_SIZE_2X:
					/* 2x */
					set_scale(X2);
					break;
				case IDM_SET_SIZE_3X:
					/* 3x */
					set_scale(X3);
					break;
				case IDM_SET_SIZE_4X:
					/* 4x */
					set_scale(X4);
					break;
				case IDM_SET_OSCAN_ON:
					set_overscan(OSCANON);
					break;
				case IDM_SET_OSCAN_OFF:
					set_overscan(OSCANOFF);
					break;
				case IDM_SET_OSCAN_DEF:
					set_overscan(OSCANDEF);
					break;
				case IDM_SET_OSCAN_DEFAULT_ON:
					set_overscan(OSCANDEFAULTON);
					break;
				case IDM_SET_OSCAN_DEFAULT_OFF:
					set_overscan(OSCANDEFAULTOFF);
					break;
				case IDM_SET_FILTER_NOFILTER:
					set_filter(NOFILTER);
					break;
				case IDM_SET_FILTER_BILINEAR:
					set_filter(BILINEAR);
					break;
#ifdef OPENGL
				case IDM_SET_FILTER_POSPHOR:
					set_filter(POSPHOR);
					break;
				case IDM_SET_FILTER_SCANLINE:
					set_filter(SCANLINE);
					break;
				case IDM_SET_FILTER_DBL:
					set_filter(DBL);
					break;
				case IDM_SET_FILTER_CRTCURVE:
					set_filter(CRTCURVE);
					break;
				case IDM_SET_FILTER_CRTNOCURVE:
					set_filter(CRTNOCURVE);
					break;
#endif
				case IDM_SET_FILTER_SCALE2X:
					set_filter(SCALE2X);
					break;
				case IDM_SET_FILTER_SCALE3X:
					set_filter(SCALE3X);
					break;
				case IDM_SET_FILTER_SCALE4X:
					set_filter(SCALE4X);
					break;
				case IDM_SET_FILTER_HQ2X:
					set_filter(HQ2X);
					break;
				case IDM_SET_FILTER_HQ3X:
					set_filter(HQ3X);
					break;
				case IDM_SET_FILTER_HQ4X:
					set_filter(HQ4X);
					break;
				case IDM_SET_FILTER_RGBNTSCCOM:
					gfx.ntscFormat = COMPOSITE;
					set_filter(RGBNTSC);
					break;
				case IDM_SET_FILTER_RGBNTSCSVD:
					gfx.ntscFormat = SVIDEO;
					set_filter(RGBNTSC);
					break;
				case IDM_SET_FILTER_RGBNTSCRGB:
					gfx.ntscFormat = RGBMODE;
					set_filter(RGBNTSC);
					break;
				case IDM_SET_PALETTE_PAL:
					gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, PALETTEPAL, FALSE);
					break;
				case IDM_SET_PALETTE_NTSC:
					gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, PALETTENTSC, FALSE);
					break;
				case IDM_SET_PALETTE_SONY:
					gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, PALETTESONY, FALSE);
					break;
				case IDM_SET_PALETTE_MONO:
					gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, PALETTEMONO, FALSE);
					break;
				case IDM_SET_PALETTE_GREEN:
					gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, PALETTEGREEN, FALSE);
					break;
#ifdef OPENGL
				case IDM_SET_RENDERING_SOFTWARE:
					set_rendering(0);
					break;
				case IDM_SET_RENDERING_OPENGL:
					set_rendering(1);
					break;
				case IDM_SET_RENDERING_GLSL:
					set_rendering(2);
					break;
				case IDM_SET_EFFECT_CUBE:
					set_effect();
					break;
				case IDM_SET_VSYNC_ON:
					set_vsync(TRUE);
					break;
				case IDM_SET_VSYNC_OFF:
					set_vsync(FALSE);
					break;
				case IDM_SET_FULLSCREEN:
					guiFullscreen();
					break;
				case IDM_SET_STRETCHFLSCR:
					opengl.aspectRatio = !opengl.aspectRatio;
					if (gfx.fullscreen == FULLSCR) {
						gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, FALSE);
					}
					guiUpdate();
					break;
#endif
				case IDM_SET_SAMPLERATE_44100:
					set_samplerate(S44100);
					break;
				case IDM_SET_SAMPLERATE_22050:
					set_samplerate(S22050);
					break;
				case IDM_SET_SAMPLERATE_11025:
					set_samplerate(S11025);
					break;
				case IDM_SET_CHANNELS_MONO:
					set_channels(MONO);
					break;
				case IDM_SET_CHANNELS_STEREO:
					set_channels(STEREO);
					break;
				case IDM_SET_AUDIO_FILTER_NONE:
					set_audio_filter(AF_NONE);
					break;
				case IDM_SET_AUDIO_FILTER_SIMPLE:
					set_audio_filter(AF_SIMPLE);
					break;
				case IDM_SET_AUDIO_ENABLE:
					emuPause(TRUE);
					cfg->audio = !cfg->audio;
					if (cfg->audio) {
						sndStart();
					} else {
						sndStop();
					}
					guiUpdate();
					emuPause(FALSE);
					break;
				case IDM_SET_GAMEGENIE:
					set_gamegenie();
					break;
				case IDM_SET_SAVENOW:
					cfgfileSave();
					break;
				case IDM_SET_SAVEONEXIT:
					cfg->saveOnExit = !cfg->saveOnExit;
					guiUpdate();
					break;
				case IDM_HELP_ABOUT:
					if (!info.portable) {
						DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd,
								(DLGPROC) aboutProc);
					} else {
						DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT_PORTABLE), hwnd,
								(DLGPROC) aboutProc);
					}
					SetFocus(hSDL);
					break;
				case IDM_SET_INPUT_CONFIG:
					cfgInput(hwnd);
					break;
			}
			break;
		}
		case WM_PAINT:
			time_handler_redraw();
			ValidateRect(hwnd, NULL);
			return(0);
		case WM_CLOSE:
			info.stop = TRUE;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS);
			return(1);
		case WM_ERASEBKGND:
			return(1);
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
long __stdcall timelineProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	BYTE dec = 0, snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);
	LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT) lParam;
	LPNMCUSTOMDRAW lpDraw;
	static char szBuf[80] = "";

	if (!tl.button) {
		SetFocus(hSDL);
	}

	switch (msg) {
		case WM_NOTIFY: {
			switch (((LPNMHDR) lParam)->code) {
				case NM_CUSTOMDRAW: {
					lpDraw = (LPNMCUSTOMDRAW) lParam;
					switch (lpDraw->dwDrawStage) {
						case CDDS_PREPAINT:
							return CDRF_NOTIFYITEMDRAW;
						case CDDS_ITEMPREPAINT: {
							switch (lpDraw->dwItemSpec) {
								case TBCD_THUMB:
									FillRect(lpDraw->hdc, &lpDraw->rc,
											(HBRUSH) GetStockObject(LTGRAY_BRUSH));
									DrawEdge(lpDraw->hdc, &lpDraw->rc, EDGE_ETCHED,
											BF_RECT | BF_ADJUST);
									return CDRF_SKIPDEFAULT;
							}
							break;
						}
					}
					break;
				}
				case TTN_SHOW:
					if (!tl.button) {
						tlDown(tl.button);
					}
					break;
				case TTN_POP:
					tlUp(tl.button);
					break;
				case TTN_NEEDTEXT:
					if (tl.snapsFill) {
						dec = ((tl.snapsFill - 1) - snap) * TLSNAPSEC;
					}
					if (!dec) {
						sprintf(szBuf, "%d sec", 0);
					} else {
						sprintf(szBuf, "% 2d sec", -abs(((tl.snapsFill - 1) - snap) * TLSNAPSEC));
					}
					lpToolTipText->lpszText = szBuf;
					break;
			}
			break;
		}
		case WM_HSCROLL:
			wrapTlPreview(snap);
			return (FALSE);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
long __stdcall saveslotProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DRAWITEMSTRUCT *pdis;
	MEASUREITEMSTRUCT *pmis;

	switch (msg) {
		case WM_MEASUREITEM: {
			pmis = (MEASUREITEMSTRUCT *) lParam;
			switch (pmis->CtlID) {
				case ID_SAVESLOT_CB: {
					TEXTMETRIC tm;
					HDC hDC = GetDC(NULL);
					GetTextMetrics(hDC, &tm);
					pmis->itemHeight = tm.tmHeight + tm.tmExternalLeading - 1;
					ReleaseDC(NULL, hDC);
					break;
				}
			}
			break;
		}
		case WM_DRAWITEM: {
			pdis = (DRAWITEMSTRUCT *) lParam;

			switch (pdis->CtlID) {
				case ID_SAVESLOT_CB: {
					HBRUSH back_brush;
					COLORREF back_colour;
					char string[120];
					int slot = 0;

					if (pdis->itemID == -1) {
						return 0;
					}

					SendMessage(hSaveslot, CB_GETLBTEXT, pdis->itemID, (LPARAM) string);

					slot = atoi(string + 5);

					if (pdis->itemState & ODS_SELECTED) {
						back_colour = RGB(0, 0, 255);
						back_brush = CreateSolidBrush(back_colour);
						FillRect(pdis->hDC, &pdis->rcItem, back_brush);
						DeleteObject(back_brush);
						SetBkColor(pdis->hDC, back_colour);
						SetTextColor(pdis->hDC, RGB(255, 255, 255));
						savestatePreview(slot);
					} else {
						back_colour = RGB(255, 255, 255);
						back_brush = CreateSolidBrush(back_colour);
						FillRect(pdis->hDC, &pdis->rcItem, back_brush);
						DeleteObject(back_brush);
						SetBkColor(pdis->hDC, back_colour);
						if (savestate.slotState[slot]) {
							SetTextColor(pdis->hDC, RGB(0, 0, 0));
						} else {
							SetTextColor(pdis->hDC, RGB(200, 200, 200));
						}
					}
					DrawText(pdis->hDC, string, strlen(string), &pdis->rcItem,
							DT_LEFT | DT_SINGLELINE);
					break;
				}
				case ID_SAVESLOT_BL:
					if (pdis->itemState & ODS_SELECTED) {
						DrawEdge(pdis->hDC, &pdis->rcItem, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
					} else {
						DrawEdge(pdis->hDC, &pdis->rcItem, EDGE_ETCHED,
								BF_MIDDLE | BF_FLAT | BF_ADJUST);
						if (pdis->itemState & ODS_DISABLED) {
							SetTextColor(pdis->hDC, GetSysColor(COLOR_GRAYTEXT));
						}
					}
					DrawText(pdis->hDC, "Load", 4, &pdis->rcItem, DT_VCENTER | DT_SINGLELINE);
					break;
				case ID_SAVESLOT_BS:
					if (pdis->itemState & ODS_SELECTED) {
						DrawEdge(pdis->hDC, &pdis->rcItem, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
					} else {
						DrawEdge(pdis->hDC, &pdis->rcItem, EDGE_ETCHED,
								BF_MIDDLE | BF_FLAT | BF_ADJUST);
						if (pdis->itemState & ODS_DISABLED) {
							SetTextColor(pdis->hDC, GetSysColor(COLOR_GRAYTEXT));
						}
					}
					DrawText(pdis->hDC, "Save", 4, &pdis->rcItem, DT_VCENTER | DT_SINGLELINE);
					break;
			}
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case ID_SAVESLOT_BS: {
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							saveslot_action(SAVE);
							break;
					}
					SetFocus(hSDL);
					break;
				}
				case ID_SAVESLOT_BL: {
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							saveslot_action(LOAD);
							break;
					}
					SetFocus(hSDL);
					break;
				}
				case ID_SAVESLOT_CB: {
					switch (HIWORD(wParam)) {
						case CBN_DROPDOWN:
							emuPause(TRUE);
							break;
						case CBN_CLOSEUP:
							savestate.slot = SendMessage(hSaveslot, CB_GETCURSEL, 0, 0);
							guiUpdate();
							savestate.previewStart = FALSE;
							emuPause(FALSE);
							SetFocus(hSDL);
							break;
						case CBN_EDITUPDATE:
							break;
						default:
							return (FALSE);
					}
					break;
				}
			}
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
long __stdcall aboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY:
			DeleteObject(aboutImg);
			DeleteObject(aboutMask);
			PostQuitMessage(0);
			break;
		case WM_PAINT: {
			BITMAP bm;
			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hwnd, &ps);

			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmOld = SelectObject(hdcMem, aboutImg);

			GetObject(aboutImg, sizeof(bm), &bm);

			SelectObject(hdcMem, aboutMask);
			BitBlt(hdc, 90, 30, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCAND);

			SelectObject(hdcMem, aboutImg);
			BitBlt(hdc, 90, 30, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCPAINT);

			SelectObject(hdcMem, hbmOld);
			DeleteDC(hdcMem);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_INITDIALOG:
			aboutImg = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ABOUT));
			aboutMask = createBitmapMask(aboutImg, RGB(255, 255, 255));
			return TRUE;
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
			}
			break;
		}
		default:
			return FALSE;
	}
	return TRUE;
}
double highResolutionMs(void) {
	uint64_t time, diff;

	QueryPerformanceCounter((LARGE_INTEGER *) &time);
	diff = ((time - gui.counterStart) * 1000) / gui.frequency;

	return ((double) (diff & 0xffffffff));
}
void open_event(void) {
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[1024];      // buffer for file name

	emuPause(TRUE);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hMainWin;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter =
	        "All supported formats\0*.nes;*.NES;*.fds;*.FDS;*.fm2;*.FM2\0"
			"Nes rom files\0*.nes;*.NES\0"
			"FDS image files\0*.fds;*.FDS\0"
			"TAS movie files\0*.fm2;*.FM2\0"
			"All files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.
	if (GetOpenFileName(&ofn) == TRUE) {
		strcpy(info.loadRomFile, ofn.lpstrFile);
		/*
		 * nascondo la finestra perche' la nuova rom potrebbe
		 * avere una configurazione dell'overscan diversa da quella
		 * della rom precedente e quindi potrei essere costretto
		 * a fare un SDL_SetVideoMode con dimensioni x ed y diverse
		 * che sotto windows, se la finestra non e' nascosta, crasha
		 * l'emulatore.
		 */
		ShowWindow(hMainWin, SW_HIDE);
		//LockWindowUpdate(hMainWin);

		gamegenie_reset(FALSE);

		make_reset(CHANGEROM);

		/* visualizzo nuovamente la finestra */
		ShowWindow(hMainWin, SW_NORMAL);
		//LockWindowUpdate(NULL);
		guiUpdate();
	}

	emuPause(FALSE);
}
void change_menuitem(BYTE checkORenab, UINT type, UINT bMenuItemID) {
	if (checkORenab == CHECK) {
		CheckMenuItem(hMainMenu, bMenuItemID, MF_BYCOMMAND | type);
	} else {
		EnableMenuItem(hMainMenu, bMenuItemID, MF_BYCOMMAND | type);
	}
}
void make_reset(BYTE type) {
	if (type == HARD) {
		if (gamegenie.enabled && gamegenie.rom_present) {
			if (info.mapper != GAMEGENIE_MAPPER) {
				strcpy(info.loadRomFile, info.romFile);
			}
			gamegenie_reset(TRUE);
			type = CHANGEROM;
		} else {
			/*
			 * se e' stato disabilitato il game genie quando ormai
			 * e' gia' in esecuzione e si preme un reset, carico la rom.
			 */
			if (info.mapper == GAMEGENIE_MAPPER) {
				gamegenie_reset(TRUE);
				type = CHANGEROM;
			}
		}
	}

	if (emuReset(type)) {
		PostMessage(hMainWin, WM_CLOSE, EXIT_FAILURE, 0);
	}
}
void set_mode(BYTE newmode) {
	BYTE reset = TRUE;

	if (newmode == cfg->mode) {
		return;
	}

	switch (newmode) {
		case PAL:
		case NTSC:
		case DENDY:
			if ((cfg->mode == AUTO) && (newmode == machine.type)) {
				reset = FALSE;
			}
			cfg->mode = newmode;
			machine = machinedb[newmode - 1];
			break;
		case AUTO:
			if (info.machineDb == PAL) {
				machine = machinedb[PAL - 1];
			} else if (info.machineDb == DENDY) {
				machine = machinedb[DENDY - 1];
			} else {
				machine = machinedb[NTSC - 1];
			}
			/*
			 * se la modalita' in cui mi trovo e' gia' quella del database oppure
			 * se ne database la modalita' e' a 0 o impostata su default ed
			 * io sono gia' nella modalita' NTSC (appunto quella di default), allora
			 * non devo fare nessun reset.
			 */
			if ((cfg->mode == info.machineDb) || ((cfg->mode == NTSC) &&
					((info.machineDb < NTSC) || (info.machineDb > DENDY)))){
				reset = FALSE;
			}
			cfg->mode = AUTO;
			break;
	}

	guiUpdate();

	if (reset) {
		textAddLineInfo(1, "switched to [green]%s", pMode[machine.type].lname);
		make_reset(CHANGEMODE);
	}
}
void set_scale(BYTE newscale) {
	if (gfx.scale == newscale) {
		return;
	}

	ShowWindow(hMainWin, SW_HIDE);

	switch (newscale) {
		case X1:
			gfxSetScreen(X1, NOCHANGE, NOCHANGE, NOCHANGE, FALSE);
			break;
		case X2:
			gfxSetScreen(X2, NOCHANGE, NOCHANGE, NOCHANGE, FALSE);
			break;
		case X3:
			gfxSetScreen(X3, NOCHANGE, NOCHANGE, NOCHANGE, FALSE);
			break;
		case X4:
			gfxSetScreen(X4, NOCHANGE, NOCHANGE, NOCHANGE, FALSE);
			break;
	}

	ShowWindow(hMainWin, SW_NORMAL);
}
void set_overscan(BYTE newoscan) {
	LockWindowUpdate(hMainWin);

	switch (newoscan) {
		case OSCANON:
		case OSCANOFF:
		case OSCANDEF:
			gfx.overscan = newoscan;
			cfgfilePgsSave();
			break;
		case OSCANDEFAULTOFF:
			gfx.overscanDefault = OSCANOFF;
			break;
		case OSCANDEFAULTON:
			gfx.overscanDefault = OSCANON;
			break;
	}

	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);

	LockWindowUpdate(NULL);
}
void set_filter(BYTE newfilter) {
	LockWindowUpdate(hMainWin);

	switch (newfilter) {
		case NOFILTER:
			gfxSetScreen(NOCHANGE, NOFILTER, NOCHANGE, NOCHANGE, FALSE);
			break;
		case BILINEAR:
			gfxSetScreen(NOCHANGE, BILINEAR, NOCHANGE, NOCHANGE, FALSE);
			break;
#ifdef OPENGL
		case POSPHOR:
			gfxSetScreen(NOCHANGE, POSPHOR, NOCHANGE, NOCHANGE, FALSE);
			break;
		case SCANLINE:
			gfxSetScreen(NOCHANGE, SCANLINE, NOCHANGE, NOCHANGE, FALSE);
			break;
		case DBL:
			gfxSetScreen(NOCHANGE, DBL, NOCHANGE, NOCHANGE, FALSE);
			break;
		case CRTCURVE:
			gfxSetScreen(NOCHANGE, CRTCURVE, NOCHANGE, NOCHANGE, FALSE);
			break;
		case CRTNOCURVE:
			gfxSetScreen(NOCHANGE, CRTNOCURVE, NOCHANGE, NOCHANGE, FALSE);
			break;
#endif
		case SCALE2X:
			gfxSetScreen(X2, SCALE2X, NOCHANGE, NOCHANGE, FALSE);
			break;
		case SCALE3X:
			gfxSetScreen(X3, SCALE3X, NOCHANGE, NOCHANGE, FALSE);
			break;
		case SCALE4X:
			gfxSetScreen(X4, SCALE4X, NOCHANGE, NOCHANGE, FALSE);
			break;
		case HQ2X:
			gfxSetScreen(X2, HQ2X, NOCHANGE, NOCHANGE, FALSE);
			break;
		case HQ3X:
			gfxSetScreen(X3, HQ3X, NOCHANGE, NOCHANGE, FALSE);
			break;
		case HQ4X:
			gfxSetScreen(X4, HQ4X, NOCHANGE, NOCHANGE, FALSE);
			break;
		case RGBNTSC:
			gfxSetScreen(NOCHANGE, RGBNTSC, NOCHANGE, NOCHANGE, FALSE);
			if (gfx.filter == RGBNTSC) {
				ntscSet(gfx.ntscFormat, 0, 0, (BYTE *) paletteRGB, 0);
				guiUpdate();
			}
			break;
	}

	LockWindowUpdate(NULL);
}
#ifdef OPENGL
void set_rendering(BYTE newrendering) {
	if ((gfx.opengl + opengl.glsl.enabled) == newrendering) {
		return;
	}

	ShowWindow(hMainWin, SW_HIDE);

	if (gfx.opengl && !newrendering) {
		opengl.rotation = FALSE;
	}

	/* switch opengl/software render */
	switch (newrendering) {
		case 0:
			gfx.opengl = FALSE;
			opengl.glsl.enabled = FALSE;
			break;
		case 1:
			gfx.opengl = TRUE;
			opengl.glsl.enabled = FALSE;
			break;
		case 2:
			gfx.opengl = TRUE;
			opengl.glsl.enabled = TRUE;
			break;
	}

	gfxResetVideo();
	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);

	ShowWindow(hMainWin, SW_NORMAL);
}
void set_vsync(BYTE bool) {
	if (gfx.vsync == bool) {
		return;
	}

	/*
	 * se non nascondo la finestra, al momento del
	 * SDL_QuitSubSystem e del SDL_InitSubSystem
	 * l'applicazione crasha.
	 */
	ShowWindow(hMainWin, SW_HIDE);

	/* switch vsync */
	gfx.vsync = bool;

	gfxResetVideo();
	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);

	ShowWindow(hMainWin, SW_NORMAL);
}
void set_effect(void) {
	if ((port1.type == ZAPPER) || (port2.type == ZAPPER)) {
		return;
	}

	opengl_unset_effect();

	opengl.rotation = !opengl.rotation;

	if (opengl.rotation) {
		opengl_init_effect = opengl_init_cube3d;
		opengl_set_effect = opengl_set_cube3d;
		opengl_unset_effect = opengl_unset_cube3d;
		opengl_draw_scene = opengl_draw_scene_cube3d;

		opengl.factorDistance = opengl.xRotate = opengl.yRotate = 0;
		if (gfx.fullscreen == FULLSCR) {
			SDL_ShowCursor(SDL_ENABLE);
		}
	} else {
		opengl_init_effect = opengl_init_no_effect;
		opengl_set_effect = opengl_set_no_effect;
		opengl_unset_effect = opengl_unset_no_effect;
		opengl_draw_scene = opengl_draw_scene_no_effect;

		if (gfx.fullscreen == FULLSCR) {
			SDL_ShowCursor(SDL_DISABLE);
		}
	}

	opengl_init_effect();

	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, FALSE);
}
#endif
void set_samplerate(int newsamplerate) {
	if (cfg->samplerate == newsamplerate) {
		return;
	}
	cfg->samplerate = newsamplerate;
	sndStart();
	guiUpdate();
}
void set_channels(int newchannels) {
	if (cfg->channels == newchannels) {
		return;
	}
	cfg->channels = newchannels;
	sndStart();
	guiUpdate();
}
void set_audio_filter(int newfilter) {
	if (cfg->audio_filter == newfilter) {
		return;
	}
	cfg->audio_filter = newfilter;
	audio_filter();
	guiUpdate();
}

void set_fps(int newfps) {
	if (cfg->fps == newfps) {
		return;
	}
	cfg->fps = newfps;
	emuPause(TRUE);
	fpsInit();
	sndStart();
	guiUpdate();
	emuPause(FALSE);
}
void set_frame_skip(int newframeskip) {
	if (cfg->frameskip == newframeskip) {
		return;
	}
	cfg->frameskip = newframeskip;
	if (!fps.fastforward) {
		fpsNormalize();
	}
	guiUpdate();
}
void set_gamegenie(void) {
	gamegenie.enabled = !gamegenie.enabled;

	if (gamegenie.enabled) {
		gamegenie_check_rom_present(TRUE);
	}

	guiUpdate();
}
void __stdcall time_handler_redraw(void) {
	gfxDrawScreen(TRUE);
}
HBITMAP createBitmapMask(HBITMAP hbmColour, COLORREF crTransparent) {
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	BITMAP bm;

	// Create monochrome (1 bit) mask bitmap.
	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	// Get some HDCs that are compatible with the display driver
	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	if (SelectBitmap(hdcMem, hbmColour) == 0) {
		;
	}
	if (SelectBitmap(hdcMem2, hbmMask) == 0) {
		;
	}

	// Set the background colour of the colour image to the colour
	// you want to be transparent.
	SetBkColor(hdcMem, crTransparent);

	// Copy the bits from the colour image to the B+W mask... everything
	// with the background colour ends up white while everythig else ends up
	// black...Just what we wanted.
	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	// Take our new mask and use it to turn the transparent colour in our
	// original colour image to black so the transparency effect will
	// work right.
	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

	// Clean up.
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	return hbmMask;
}
void wrapTlPreview(BYTE snap) {
	if (!(tl.button | tl.key)) {
		return;
	}

	if (!tl.snapsFill) {
		SendMessage(hTimeline, TBM_SETPOS, TRUE, 0);
		return;
	}

	/* snap non puo' essere mai maggiore del numero di snap effettuate */
	if (snap > (tl.snapsFill - 1)) {
		SendMessage(hTimeline, TBM_SETPOS, TRUE, tl.snapsFill - 1);
		snap = (tl.snapsFill - 1);
	}

	SendMessage(hTimeline, TBM_SETPOS, TRUE, snap);

	if (snap == (tl.snapsFill - 1)) {
		memcpy(screen.data, tl.snaps[TLSNAPFREE] + tl.preview, screenSize());
		time_handler_redraw();
		return;
	}

	timelinePreview(snap);
}
void saveslot_incdec(BYTE mode) {
	BYTE newslot;

	if (mode == INC) {
		newslot = savestate.slot + 1;
		if (newslot >= SSAVAILABLE) {
			newslot = 0;
		}
	} else {
		newslot = savestate.slot - 1;
		if (newslot >= SSAVAILABLE) {
			newslot = SSAVAILABLE - 1;
		}
	}
	saveslot_set(newslot);
}
void saveslot_action(BYTE mode) {
	emuPause(TRUE);

	if (mode == SAVE) {
		savestateSave();
		cfgfilePgsSave();
	} else {
		savestateLoad();
	}

	guiUpdate();

	emuPause(FALSE);
}
void saveslot_set(BYTE selection) {
	SendMessage(hSaveslot, CB_SETCURSEL, selection, 0);
	savestate.slot = selection;
	guiUpdate();
}
void fds_eject_insert_disk(void) {
	if (!fds.drive.disk_ejected) {
		fds_disk_op(FDS_DISK_EJECT, 0);
	} else {
		fds_disk_op(FDS_DISK_INSERT, 0);
	}

	guiUpdate();
}
void fds_select_side(int side) {
	if (side == 0xFFF) {
		side = fds.drive.side_inserted;
		if (++side >= fds.info.total_sides) {
			side = 0;
		}
	}

	if (fds.drive.side_inserted == side) {
		return;
	}

	fds_disk_op(FDS_DISK_SELECT, side);

	guiUpdate();
}
