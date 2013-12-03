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
#include "input.h"
#include "overscan.h"
#include "gfx.h"
#include "snd.h"
#include "clock.h"
#include "cfg_file.h"
#include "timeline.h"
#include "save_slot.h"
#include "version.h"
#include "fps.h"
#include "tas.h"
#include "text.h"
#include "param.h"
#include "ppu.h"
#include "fds.h"
#include "gamegenie.h"
#include "audio_quality.h"
#include "opengl.h"
#include "openGL/no_effect.h"
#include "openGL/cube3d.h"
#include "recent_roms.h"
#include "cfg_input.h"
#include "cfg_apu_channels.h"

#define timer_redraw_start()\
	SetTimer(hwnd, IDT_TIMER1, 650, (TIMERPROC) time_handler_redraw)
#define timer_redraw_stop()\
	KillTimer(hwnd, IDT_TIMER1)
#define tl_down(type)\
	emu_pause(TRUE);\
	type = TRUE;\
	if (tl.snaps_fill) {\
		/* faccio lo screenshot dello screen attuale */\
		memcpy(tl.snaps[TL_SNAP_FREE] + tl.preview, screen.data, screen_size());\
	}
#define tl_up(type)\
	if (tl.snaps_fill) {\
		BYTE snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);\
		if (snap != (tl.snaps_fill - 1)) {\
			timeline_back(TL_NORMAL, snap);\
		}\
	}\
	SetFocus(sdl_frame);\
	type = FALSE;\
	emu_pause(FALSE)
#define hide_tool_widget()\
	ShowWindow(hFrameSs, SW_HIDE)
#define show_tool_widget()\
	ShowWindow(hFrameSs, SW_SHOW)

enum { INC, DEC };
enum { SAVE, LOAD };
enum { CHECK, ENAB };

#define TOOLBAR_HEIGHT   26
#define FRAME_TL_HEIGHT  (TOOLBAR_HEIGHT - 2)
#define FRAME_TL_WIDTH   SCR_ROWS
#define FRAME_SS_HEIGHT  TOOLBAR_HEIGHT - 2
#define FRAME_SS_WIDTH   1 + BUTTON_SS_WIDTH + 0 + COMBO_SS_WIDTH + 2 + BUTTON_SS_WIDTH + 1
#define BUTTON_SS_HEIGHT FRAME_SS_HEIGHT - 1
#define BUTTON_SS_WIDTH  31
#define COMBO_SS_WIDTH   60
#define SEPARATOR_WIDTH  3

LRESULT CALLBACK cbt_proc(int code, WPARAM wParam, LPARAM lParam);
long __stdcall main_win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall timeline_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall save_slot_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall about_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
double high_resolution_ms(void);
void open_event(void);
void change_menuitem(BYTE check_or_enab, UINT type, UINT menuitem_id);
void make_reset(BYTE type);
void set_mode(BYTE mode);
void set_scale(BYTE scale);
void set_overscan(BYTE oscan);
void set_filter(BYTE filter);
void set_rendering(BYTE rendering);
void set_vsync(BYTE vsync);
void set_samplerate(int samplerate);
void set_channels(int channels);
void set_stereo_delay(int stereo_delay);
void set_audio_quality(int quality);
void set_fps(int fps);
void set_frame_skip(int frameskip);
void set_gamegenie(void);
void __stdcall time_handler_redraw(void);
HBITMAP create_bitmap_mask(HBITMAP hbm_colour, COLORREF cr_transparent);
void wrap_tl_preview(BYTE snap);
void save_slot_incdec(BYTE mode);
void save_slot_action(BYTE mode);
void save_slot_set(BYTE selection);
void fds_eject_insert_disk(void);
void fds_select_side(int side);
void change_rom(char *rom);

static HHOOK msgbox_hook;
static HWND main_win, sdl_frame, toolbox_frame;
static HWND hFrameTl, hTimeline;
static HWND hSepTl;
static HWND hFrameSs, hSaveslot, hSaveButton, hLoadButton;
static HWND hSepSs;
static HWND hFrameBl;
HMENU main_menu;
HACCEL acc_keys;
HBITMAP about_img, about_mask;
MONITORINFO mi = { sizeof(mi) };
WINDOWPLACEMENT wp_prev = { sizeof(wp_prev) };

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
			gui_get_ms = sdl_get_ms;
		} else {
			gui.frequency = (double) pf;
			QueryPerformanceCounter((LARGE_INTEGER *) &pf);
			gui.counter_start = pf;
			gui_get_ms = high_resolution_ms;
		}
	}

	gui.accelerators_anabled = TRUE;

	gui.richedit = LoadLibrary("Riched20.Dll");
}
void gui_quit(void) {
	DestroyWindow(main_win);

	FreeLibrary(gui.richedit);
}
BYTE gui_create(void) {
	WNDCLASSEX wc;
	INITCOMMONCONTROLSEX icex;
	const char className[] = "FHWindowClass";

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) main_win_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = gui.main_hinstance;
	wc.hIcon = LoadIcon(gui.main_hinstance, MAKEINTRESOURCE(IDI_MYICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = className;
	wc.hIconSm = LoadIcon(gui.main_hinstance, MAKEINTRESOURCE(IDI_MYICON));

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* creo la finestra principale */
	main_win = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_ACCEPTFILES,
			className,
			"puNES window",
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

	main_menu = GetMenu(main_win);

	acc_keys = LoadAccelerators(gui.main_hinstance, "IDR_ACCKEYS");

	if (acc_keys == NULL) {
		MessageBox(NULL, "Window Accelerators Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- SDL window ---------------------------------- */
	/* creo la finestra a cui attacchero' lo screen sdl */
	sdl_frame = CreateWindowEx(0, className, "puNES SDL Frame", WS_CHILD, CW_USEDEFAULT,
	        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, main_win, (HMENU) NULL, gui.main_hinstance,
	        NULL);

	if (sdl_frame == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- Toolbar ---------------------------------- */

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);

	toolbox_frame = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE,
	        CW_USEDEFAULT, CW_USEDEFAULT, 0, TOOLBAR_HEIGHT, main_win, (HMENU) NULL,
	        gui.main_hinstance, NULL);

	if (toolbox_frame == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	/* ---------------------------------- Timeline ---------------------------------- */

	/* Frame Timeline */
	hFrameTl = CreateWindowEx(0, "static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, 0, 0,
	        FRAME_TL_WIDTH, FRAME_TL_HEIGHT, toolbox_frame, NULL, NULL, NULL);

	if (hFrameTl == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SetWindowLongPtr(hFrameTl, GWLP_WNDPROC, (LONG_PTR) timeline_proc);

	hTimeline = CreateWindowEx(0, TRACKBAR_CLASS, "Timeline",
	        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_FIXEDLENGTH | TBS_TOOLTIPS, 2, 0,
	        FRAME_TL_WIDTH - 4, FRAME_TL_HEIGHT, hFrameTl, (HMENU) NULL, gui.main_hinstance, NULL);

	if (hTimeline == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hTimeline, TBM_SETRANGE, TRUE, MAKELONG(0, (TL_SNAPS - 1)));
	SendMessage(hTimeline, TBM_SETTHUMBLENGTH, 15, 0);
	SendMessage(hTimeline, TBM_SETPAGESIZE, 0, 0);
	SendMessage(hTimeline, TBM_SETTIPSIDE, TBTS_BOTTOM, 0);

	/* -------------------------------- Separatore Tl -------------------------------- */

	hSepTl = CreateWindowEx(0, "static", "", WS_CHILD | WS_VISIBLE, 0, 0, SEPARATOR_WIDTH,
	        TOOLBAR_HEIGHT, toolbox_frame, (HMENU) NULL, gui.main_hinstance, NULL);

	/* ---------------------------------- Save slot ---------------------------------- */

	HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

	/* Frame Saveslot */
	hFrameSs = CreateWindowEx(WS_EX_WINDOWEDGE, "static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
	        0, 0, FRAME_SS_WIDTH, FRAME_SS_HEIGHT, toolbox_frame, NULL, NULL, NULL);

	if (hFrameSs == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SetWindowLongPtr(hFrameSs, GWLP_WNDPROC, (LONG_PTR) save_slot_proc);

	hSaveslot = CreateWindowEx(0, "COMBOBOX", "Saveslot",
	        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_OWNERDRAWFIXED,
	        1 + BUTTON_SS_WIDTH + 0, 1, COMBO_SS_WIDTH, 130, hFrameSs, (HMENU) ID_SAVE_SLOT_CB,
	        gui.main_hinstance, NULL);

	if (hSaveslot == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hSaveslot, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

	hSaveButton = CreateWindowEx(0,
			"BUTTON",
			"Save",
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			1, 0,
	        BUTTON_SS_WIDTH, BUTTON_SS_HEIGHT,
	        hFrameSs,
	        (HMENU) ID_SAVE_SLOT_BS,
	        gui.main_hinstance,
	        NULL);

	if (hSaveButton == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hSaveButton, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

	hLoadButton = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
	        1 + BUTTON_SS_WIDTH + 0 + COMBO_SS_WIDTH + 2, 0, BUTTON_SS_WIDTH,
	        BUTTON_SS_HEIGHT, hFrameSs, (HMENU) ID_SAVE_SLOT_BL, gui.main_hinstance, NULL);

	if (hLoadButton == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	SendMessage(hLoadButton, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));

	{
		BYTE i;

		for (i = 0; i < SAVE_SLOTS; i++) {
			char item[10];
			sprintf(item, "Slot %d", i);
			SendMessage(hSaveslot, CB_ADDSTRING, 0, (LPARAM) item);
		}
	}

	SendMessage(hSaveslot, CB_SETCURSEL, 0, 0);

	/* -------------------------------- Separatore Ss -------------------------------- */

	hSepSs = CreateWindowEx(0, "static", "", WS_CHILD | WS_VISIBLE, 0, 0, SEPARATOR_WIDTH,
	        TOOLBAR_HEIGHT, toolbox_frame, (HMENU) NULL, gui.main_hinstance, NULL);

	/* -------------------------------- Frame vuoto -------------------------------- */

	/* Frame Saveslot */
	hFrameBl = CreateWindowEx(WS_EX_WINDOWEDGE, "static", "", WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT,
	        0, 0, 0, FRAME_SS_HEIGHT, toolbox_frame, NULL, NULL, NULL);

	if (hFrameBl == NULL) {
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
	MoveWindow(sdl_frame, 0, 0, gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE], TRUE);
	/* aggiorno la toolbar */
	MoveWindow(toolbox_frame, 0, gfx.h[VIDEO_MODE], gfx.w[VIDEO_MODE], TOOLBAR_HEIGHT, TRUE);
	/* aggiorno il frame della timeline */
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
	pt_diff.x -= SEPARATOR_WIDTH;
	MoveWindow(hSepTl, pt_diff.x, 0, SEPARATOR_WIDTH, FRAME_TL_HEIGHT, TRUE);
	/* aggiorno il frame dello saveslot */
	pt_diff.x -= FRAME_SS_WIDTH;
	MoveWindow(hFrameSs, pt_diff.x, 0, FRAME_SS_WIDTH, FRAME_SS_HEIGHT, TRUE);
	pt_diff.x -= SEPARATOR_WIDTH;
	MoveWindow(hSepSs, pt_diff.x, 0, SEPARATOR_WIDTH, FRAME_TL_HEIGHT, TRUE);
	/* frame vuoto */
	pt_diff.y = gfx.w[VIDEO_MODE] - pt_diff.x;
	MoveWindow(hFrameBl, 0, 0, gfx.w[VIDEO_MODE] - pt_diff.y, FRAME_TL_HEIGHT, TRUE);
}
void gui_start(void) {
	/* visualizzo il frame principale */
	ShowWindow(main_win, SW_NORMAL);
	UpdateWindow(main_win);
	/* visualizzo il frame sdl */
	ShowWindow(sdl_frame, SW_SHOWNOACTIVATE);
	UpdateWindow(sdl_frame);
	/* visualizzo la toolbar */
	ShowWindow(toolbox_frame, SW_SHOWNOACTIVATE);
	UpdateWindow(toolbox_frame);
	/* visualizzo il frame della timeline */
	ShowWindow(hFrameTl, SW_SHOWNOACTIVATE);
	UpdateWindow(hFrameTl);
	/* visualizzo la timeline */
	ShowWindow(hTimeline, SW_SHOWNOACTIVATE);
	UpdateWindow(hTimeline);

	/* setto il focus sulla finestra sdl */
	SetForegroundWindow(sdl_frame);
	SetFocus(sdl_frame);

	gui.start = TRUE;

	emu_loop();
	return;
}
void gui_event(void) {
	BYTE no_process = FALSE;
	MSG msg = { 0 };

	/* SDL */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		/*
		 * e' a zero solo quando non ci sono messaggi
		 * e info.pause e' settato.
		 */
		if (!msg.hwnd) {
			Sleep(3);
			continue;
		}
		//fprintf(stderr, "0: %X %X\n", Msg.message, LOWORD(Msg.wParam));
		switch (msg.message) {
			case WM_KEYDOWN: {
				switch (LOWORD(msg.wParam)) {
					case VK_CONTROL:
						if (!tl.key) {
							tl_down(tl.key);
						}
						no_process = TRUE;
						break;
					case VK_SHIFT:
						fps_fast_forward();
						no_process = TRUE;
						break;
					case VK_LEFT:
						if (tl.key) {
							BYTE snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);

							if (snap) {
								wrap_tl_preview(snap - 1);
							}
							no_process = TRUE;
						}
						break;
					case VK_RIGHT:
						if (tl.key) {
							BYTE snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);

							if (snap < (TL_SNAPS - 1)) {
								wrap_tl_preview(snap + 1);
							}
							no_process = TRUE;
						}
						break;
					case VK_ESCAPE:
						if (cfg->fullscreen == FULLSCR) {
							gui_fullscreen();
						}
						no_process = TRUE;
						break;
				}
				if (!tas.type && !no_process) {
					BYTE i;

					for (i = PORT1; i < PORT_MAX; i++) {
						if (input_decode_event[i]) {
							input_decode_event[i](PRESSED, LOWORD(msg.wParam), KEYBOARD, &port[i]);
						}
					}
				}
				break;
			}
			case WM_KEYUP: {
				switch (LOWORD(msg.wParam)) {
					case VK_CONTROL:
						if (tl.key) {
							tl_up(tl.key);
						}
						no_process = TRUE;
						break;
					case VK_SHIFT:
						fps_normalize();
						no_process = TRUE;
						break;
				}
				if (!tas.type && !no_process) {
					BYTE i;

					for (i = PORT1; i < PORT_MAX; i++) {
						if (input_decode_event[i]) {
					        input_decode_event[i](RELEASED, LOWORD(msg.wParam), KEYBOARD, &port[i]);
						}
					}
				}
				break;
			}
			case WM_LBUTTONDOWN:
				gui.left_button = TRUE;
				//opengl.x_diff = GET_X_LPARAM(Msg.lParam) - (opengl.y_rotate * slow_factor);
				//opengl.y_diff = -GET_Y_LPARAM(Msg.lParam) + (opengl.x_rotate * slow_factor);
				opengl.x_diff = gui.x - (opengl.y_rotate * slow_factor);
				opengl.y_diff = -gui.y + (opengl.x_rotate * slow_factor);
				break;
			case WM_RBUTTONDOWN:
				gui.right_button = TRUE;
				break;
			case WM_MOUSEMOVE:
				if (msg.hwnd == sdl_frame) {
					gui.x = GET_X_LPARAM(msg.lParam);
					gui.y = GET_Y_LPARAM(msg.lParam);
				}
				if (gui.left_button && opengl.rotation) {
					opengl.x_rotate = (gui.y + opengl.y_diff) / slow_factor;
					opengl.y_rotate = (gui.x - opengl.x_diff) / slow_factor;
				}
				break;
			case WM_LBUTTONUP:
				gui.left_button = FALSE;
				break;
			case WM_RBUTTONUP:
				gui.right_button = FALSE;
				break;
		}
		if ((gui.accelerators_anabled == FALSE)
		        || !TranslateAccelerator(main_win, acc_keys, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		msg.hwnd = 0;
	}

	if (info.no_rom | info.pause) {
		return;
	}

	if (tas.type) {
		tas_frame();
		return;
	}

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (input_add_event[i]) {
				input_add_event[i](i);
			}
		}
	}

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (input_add_event[i]) {
				input_add_event[i](i);
			}
		}
	}
}
HWND gui_emu_frame_id(void) {
	return (sdl_frame);
}
void gui_update(void) {
	WORD id = 0;
	char title[255];

	/* aggiorno il titolo */
	emu_set_title(title);
	SetWindowText(main_win, title);

	/* aggiorno il menu dei files aperti di recente */
	{
		HMENU menu_file = GetSubMenu(main_menu, 0), menu_recent;
		MENUITEMINFO mr_mi = { 0 };
		UINT index = 0;

		DeleteMenu(menu_file, 1, MF_BYPOSITION);
		menu_recent = CreatePopupMenu();

		mr_mi.cbSize = sizeof(MENUITEMINFO);
		mr_mi.fMask = MIIM_STRING | MIIM_ID;
		if (recent_roms_list.count > 0) {
			mr_mi.fMask |= MIIM_SUBMENU;
		}
		mr_mi.wID = IDM_FILE_RECENT;
		mr_mi.hSubMenu = menu_recent;
		mr_mi.dwTypeData = "Recent Roms";

		InsertMenuItem(menu_file, 0, FALSE, &mr_mi);

		if (recent_roms_list.count > 0) {
			for (index = 0; index < RECENT_ROMS_MAX; index++) {
				MENUITEMINFO mi = { 0 };
				char description[RECENT_ROMS_LINE];

				if (recent_roms_list.item[index][0] == 0) {
					break;
				}

				mi.cbSize = sizeof(MENUITEMINFO);
				mi.fMask = MIIM_STRING | MIIM_ID;
				mi.wID = IDM_FILE_RECENT_0 + index;

				sprintf(description, "%s", basename(recent_roms_list.item[index]));
				mi.dwTypeData = description;

				InsertMenuItem(menu_recent, index, FALSE, &mi);
			}
		}
	}

	/* checko le voci di menu corrette */

	/* FDS */
	if (fds.info.enabled) {
		BYTE i;

		HMENU menu_NES = GetSubMenu(main_menu, 1);
		MENUITEMINFO menuitem;

		if (fds.drive.disk_ejected) {
			menuitem.dwTypeData = "&Insert disk\tALT+E";
		} else {
			menuitem.dwTypeData = "&Eject disk\tALT+E";
		}
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STRING | MIIM_STATE;
		menuitem.fState = MFS_ENABLED;
		SetMenuItemInfo(menu_NES, IDM_NES_FDS_EJECT, FALSE, &menuitem);

		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_ENABLED;
		SetMenuItemInfo(menu_NES, 3, TRUE, &menuitem);

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
		HMENU menu_NES = GetSubMenu(main_menu, 1);
		MENUITEMINFO menuitem;

		menuitem.dwTypeData = "Ej&ect/Insert disk\tALT+E";
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STRING | MIIM_STATE;
		menuitem.fState = MFS_DISABLED;
		SetMenuItemInfo(menu_NES, IDM_NES_FDS_EJECT, FALSE, &menuitem);

		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_GRAYED;
		SetMenuItemInfo(menu_NES, 3, TRUE, &menuitem);

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
	switch (save_slot.slot) {
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
	if (save_slot.state[save_slot.slot]) {
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
		case FPS_DEFAULT:
			id = IDM_SET_FPS_DEFAULT;
			break;
		case FPS_60:
			id = IDM_SET_FPS_60;
			break;
		case FPS_59:
			id = IDM_SET_FPS_59;
			break;
		case FPS_58:
			id = IDM_SET_FPS_58;
			break;
		case FPS_57:
			id = IDM_SET_FPS_57;
			break;
		case FPS_56:
			id = IDM_SET_FPS_56;
			break;
		case FPS_55:
			id = IDM_SET_FPS_55;
			break;
		case FPS_54:
			id = IDM_SET_FPS_54;
			break;
		case FPS_53:
			id = IDM_SET_FPS_53;
			break;
		case FPS_52:
			id = IDM_SET_FPS_52;
			break;
		case FPS_51:
			id = IDM_SET_FPS_51;
			break;
		case FPS_50:
			id = IDM_SET_FPS_50;
			break;
		case FPS_49:
			id = IDM_SET_FPS_49;
			break;
		case FPS_48:
			id = IDM_SET_FPS_48;
			break;
		case FPS_47:
			id = IDM_SET_FPS_47;
			break;
		case FPS_46:
			id = IDM_SET_FPS_46;
			break;
		case FPS_45:
			id = IDM_SET_FPS_45;
			break;
		case FPS_44:
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
	if (cfg->filter != NO_FILTER) {
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_SIZE_1X);
	} else {
		change_menuitem(ENAB, MF_HILITE, IDM_SET_SIZE_1X);
	}
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_1X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_2X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_3X);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SIZE_4X);
	if (cfg->fullscreen == NO_FULLSCR) {
		switch (cfg->scale) {
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
	switch (cfg->oscan) {
		case OSCAN_ON:
			id = IDM_SET_OSCAN_ON;
			break;
		case OSCAN_OFF:
			id = IDM_SET_OSCAN_OFF;
			break;
		case OSCAN_DEFAULT:
			id = IDM_SET_OSCAN_DEF;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Overscan/Default value */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_DEFAULT_ON);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_OSCAN_DEFAULT_OFF);
	switch (cfg->oscan_default) {
		case OSCAN_ON:
			id = IDM_SET_OSCAN_DEFAULT_ON;
			break;
		case OSCAN_OFF:
			id = IDM_SET_OSCAN_DEFAULT_OFF;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Filter */
	if (gfx.bit_per_pixel < 32) {
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_HQ2X);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_HQ3X);
		change_menuitem(ENAB, MF_GRAYED, IDM_SET_FILTER_HQ4X);
	} else {
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_HQ2X);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_HQ3X);
		change_menuitem(ENAB, MF_ENABLED, IDM_SET_FILTER_HQ4X);
	}
	{
		HMENU menu_to_change;
		MENUITEMINFO menuitem;

		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;

		/* Video/Rendering */
		menu_to_change = GetSubMenu(GetSubMenu(GetSubMenu(main_menu, 2), 2), 0);

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

		menu_to_change = GetSubMenu(GetSubMenu(GetSubMenu(main_menu, 2), 2), 7);

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
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_NO_FILTER);
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
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_POSPHOR);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_SCANLINE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_DBL);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_CRTCURVE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_FILTER_CRTNOCURVE);
	switch (cfg->filter) {
		case NO_FILTER:
			id = IDM_SET_FILTER_NO_FILTER;
			break;
		case BILINEAR:
			id = IDM_SET_FILTER_BILINEAR;
			break;
		case POSPHOR:
			id = IDM_SET_FILTER_POSPHOR;
			break;
		case SCANLINE:
			id = IDM_SET_FILTER_SCANLINE;
			break;
		case DBL:
			id = IDM_SET_FILTER_DBL;
			break;
		case CRT_CURVE:
			id = IDM_SET_FILTER_CRTCURVE;
			break;
		case CRT_NO_CURVE:
			id = IDM_SET_FILTER_CRTNOCURVE;
			break;
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
		case NTSC_FILTER:
			switch (cfg->ntsc_format) {
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
	switch (cfg->palette) {
		case PALETTE_PAL:
			id = IDM_SET_PALETTE_PAL;
			break;
		case PALETTE_NTSC:
			id = IDM_SET_PALETTE_NTSC;
			break;
		case PALETTE_SONY:
			id = IDM_SET_PALETTE_SONY;
			break;
		case PALETTE_MONO:
			id = IDM_SET_PALETTE_MONO;
			break;
		case PALETTE_GREEN:
			id = IDM_SET_PALETTE_GREEN;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_RENDERING_SOFTWARE);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_RENDERING_OPENGL);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_RENDERING_GLSL);
	if (gfx.opengl) {
		HMENU menuSettings = GetSubMenu(main_menu, 2);
		HMENU menuVideo = GetSubMenu(menuSettings, 2);
		MENUITEMINFO menuitem;

		/* VSync */
		menuitem.cbSize = sizeof(MENUITEMINFO);
		menuitem.fMask = MIIM_STATE;
		menuitem.fState = MFS_ENABLED;
		SetMenuItemInfo(menuVideo, 3, TRUE, &menuitem);

		/* questi li abilito solo se non c'e' come input lo zapper */

		if (input_zapper_is_connected((_port *) &port) == FALSE) {
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
		HMENU menuSettings = GetSubMenu(main_menu, 2);
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
	if (cfg->vsync) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_VSYNC_ON);
	} else {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_VSYNC_OFF);
	}

	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_STRETCHFLSCR);
	if (gfx.opengl && !cfg->aspect_ratio) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_STRETCHFLSCR);
	}

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
		case MONO:{
			HMENU menuChannels = GetSubMenu(GetSubMenu(GetSubMenu(main_menu, 2), 3), 1);
			MENUITEMINFO menuitem;

			/* Stereo delay */
			menuitem.cbSize = sizeof(MENUITEMINFO);
			menuitem.fMask = MIIM_STATE;
			menuitem.fState = MFS_DISABLED;
			SetMenuItemInfo(menuChannels, 3, TRUE, &menuitem);

			id = IDM_SET_CHANNELS_MONO;
			break;
		}
		case STEREO: {
			HMENU menuChannels = GetSubMenu(GetSubMenu(GetSubMenu(main_menu, 2), 3), 1);
			MENUITEMINFO menuitem;

			/* Stereo delay */
			menuitem.cbSize = sizeof(MENUITEMINFO);
			menuitem.fMask = MIIM_STATE;
			menuitem.fState = MFS_ENABLED;
			SetMenuItemInfo(menuChannels, 3, TRUE, &menuitem);

			id = IDM_SET_CHANNELS_STEREO;
			break;
		}
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Stereo delay */
	{
		int index;

		for (index = IDM_SET_STEREO_DELAY_5; index <= IDM_SET_STEREO_DELAY_100; index++) {
			int delay = cfg->stereo_delay * 100;

			if (delay == (((index - IDM_SET_STEREO_DELAY_5) + 1) * 5)) {
				change_menuitem(CHECK, MF_CHECKED, index);
			} else {
				change_menuitem(CHECK, MF_UNCHECKED, index);
			}
		}
	}

	/* Audio Filter */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_QUALITY_LOW);
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_QUALITY_HIGH);
	switch (cfg->audio_quality) {
		case AQ_LOW:
			id = IDM_SET_AUDIO_QUALITY_LOW;
			break;
		case AQ_HIGH:
			id = IDM_SET_AUDIO_QUALITY_HIGH;
			break;
	}
	change_menuitem(CHECK, MF_CHECKED, id);

	/* Swap Duty Cycles */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_SWAP_DUTY);
	if (cfg->swap_duty) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_AUDIO_SWAP_DUTY);
	}

	/* Audio Enable */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_AUDIO_ENABLE);
	if (cfg->apu.channel[APU_MASTER]) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_AUDIO_ENABLE);
	}

	/* Game Genie */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_GAMEGENIE);
	if (cfg->gamegenie) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_GAMEGENIE);
	}

	/* Save on exit */
	change_menuitem(CHECK, MF_UNCHECKED, IDM_SET_SAVEONEXIT);
	if (cfg->save_on_exit) {
		change_menuitem(CHECK, MF_CHECKED, IDM_SET_SAVEONEXIT);
	}
}
void gui_fullscreen(void) {
	emu_pause(TRUE);

	/* nascondo la finestra */
	ShowWindow(main_win, SW_HIDE);

	if ((cfg->fullscreen == NO_FULLSCR) || (cfg->fullscreen == NO_CHANGE)) {
		/* salvo il valore scale prima del fullscreen */
		gfx.scale_before_fscreen = cfg->scale;
		/* trovo la risoluzione del monitor in uso */
		GetMonitorInfo(MonitorFromWindow(main_win, MONITOR_DEFAULTTOPRIMARY), &mi);
		gfx.w[MONITOR] = mi.rcMonitor.right - mi.rcMonitor.left;
		gfx.h[MONITOR] = mi.rcMonitor.bottom - mi.rcMonitor.top;
		/*salvo la posizione della finestra */
		if (gui.start) {
			GetWindowPlacement(main_win, &wp_prev);
		} else {
			wp_prev.rcNormalPosition.bottom = 0;
		}
		/* dissocio il menu dalla finestra */
		SetMenu(main_win, NULL);
		/* abilito il fullscreen */
		gfx_set_screen(NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE);
		/* disabilito la visualizzazione del puntatore */
		if (!opengl.rotation && (input_zapper_is_connected((_port *) &port) == FALSE)) {
			SDL_ShowCursor(SDL_DISABLE);
		}
		/* queste sono le cose che devo disabilitare per il fullscreen */
		SetWindowLongPtr(main_win, GWL_STYLE,
		        GetWindowLong(main_win, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER | WS_SYSMENU));
		SetWindowLongPtr(main_win, GWL_EXSTYLE,
		        (GetWindowLongPtr(main_win, GWL_EXSTYLE) | WS_EX_APPWINDOW | WS_EX_TOPMOST)
		                & ~WS_EX_CLIENTEDGE);
		/* muovo la finestra al margine superiore destro del monitor */
		MoveWindow(main_win, mi.rcMonitor.left, mi.rcMonitor.top, gfx.w[VIDEO_MODE],
		        gfx.h[VIDEO_MODE], TRUE);
	} else {
		/* ribilito gli stili della finestra corretti */
		SetWindowLongPtr(main_win, GWL_STYLE,
		        GetWindowLong(main_win, GWL_STYLE) | WS_CAPTION | WS_BORDER | WS_SYSMENU);
		SetWindowLongPtr(main_win, GWL_EXSTYLE,
		        (GetWindowLongPtr(main_win, GWL_EXSTYLE) | WS_EX_CLIENTEDGE)
		                & ~(WS_EX_APPWINDOW | WS_EX_TOPMOST));
		/* riassocio il menu */
		SetMenu(main_win, main_menu);
		/* ripristino i valori di scale ed esco dal fullscreen */
		gfx_set_screen(gfx.scale_before_fscreen, NO_CHANGE, NO_FULLSCR, NO_CHANGE, FALSE);
		/* riabilito la visualizzazione del puntatore */
		SDL_ShowCursor(SDL_ENABLE);
		/* posiziono la finestra alle coordinate precedenti il fullscreen */
		if (wp_prev.rcNormalPosition.bottom) {
			SetWindowPlacement(main_win, &wp_prev);
		}
	}

	/* visualizzo la finestra */
	ShowWindow(main_win, SW_NORMAL);
	/* setto il focus*/
	SetFocus(sdl_frame);

	emu_pause(FALSE);
}
void gui_timeline(void) {
	SendMessage(hTimeline, TBM_SETPOS, TRUE, tl.snaps_fill - 1);
}
void gui_save_slot(BYTE slot) {
	if (slot >= SAVE_SLOTS) {
		slot = SAVE_SLOTS - 1;
	}
	save_slot_set(slot);
}
void gui_sleep(double ms) {
	if (ms > 0) {
		Sleep(ms);
	}
}
void gui_set_thread_affinity(uint8_t core) {
	SetThreadAffinityMask(GetCurrentThread(), core + 1);
}
void gui_print_usage(char *usage) {
	msgbox_hook = SetWindowsHookEx(WH_CBT, cbt_proc, NULL, GetCurrentThreadId());
	MessageBox(NULL, usage, NAME " parameters", MB_OK);
	UnhookWindowsHookEx(msgbox_hook);
}
void gui_reset_video(void) {
	ShowWindow(main_win, SW_HIDE);

	gfx_reset_video();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);

	ShowWindow(main_win, SW_NORMAL);
}

/* funzioni interne */
LRESULT CALLBACK cbt_proc(int code, WPARAM wParam, LPARAM lParam) {
	static HFONT font = NULL;
	static HWND txt = NULL, button = NULL;
	static RECT rc_button;

	if (code < 0) {
		return (CallNextHookEx(msgbox_hook, code, wParam, lParam));
	}

	switch (code) {
		case HCBT_CREATEWND: {
			HWND hwnd = (HWND) wParam;
			TCHAR szClassName[16];

			if (GetClassName(hwnd, szClassName, 16)) {
				if (strcmp(szClassName, "Static") == 0) {
					txt = hwnd;
				} else if (strcmp(szClassName, "Button") == 0) {
					button = hwnd;
				}
			}
			break;
		}
		case HCBT_ACTIVATE: {
			HWND hwnd = (HWND) wParam;
			RECT rc_client, rc_wind;
			POINT pt_diff;

			/* aggiorno la dimensione della finestra principale */
			GetWindowRect(hwnd, &rc_wind);
			GetClientRect(hwnd, &rc_client);

			pt_diff.x = (rc_wind.right - rc_wind.left) - rc_client.right;
			pt_diff.y = (rc_wind.bottom - rc_wind.top) - rc_client.bottom;

			{
				INT x, y, widht_font = 13;

				font = CreateFont(widht_font, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
						ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, TEXT("Monospace"));

				SendMessage(txt, WM_SETFONT,(WPARAM) font, TRUE);
				RedrawWindow(txt, NULL, NULL, RDW_UPDATENOW);

#define BORDER_SIZE 2
				x = (50 * widht_font);
				y = (30 * widht_font);

				if (rc_button.bottom == 0) {
					GetClientRect(button, &rc_button);
				}

				pt_diff.x += x;
				pt_diff.y += y + (rc_button.bottom + (BORDER_SIZE * 2));

				MoveWindow(hwnd, rc_wind.left, rc_wind.top, pt_diff.x, pt_diff.y, TRUE);
				MoveWindow(txt, 0, 0, x, y, TRUE);
				MoveWindow(button, BORDER_SIZE, y + BORDER_SIZE, x - (BORDER_SIZE * 2),
						rc_button.bottom, TRUE);
#undef BORDER_SIZE
			}

			return (0);
		}
		case HCBT_DESTROYWND:
			DeleteObject(font);
			return (0);
	}

	return (CallNextHookEx(msgbox_hook, code, wParam, lParam));
}
long __stdcall main_win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_ENTERSIZEMOVE:
		case WM_ENTERMENULOOP:
			emu_pause(TRUE);
			//timer_redraw_start();
			break;
		case WM_EXITSIZEMOVE:
		case WM_EXITMENULOOP:
			emu_pause(FALSE);
			//timer_redraw_stop();
			SetFocus(sdl_frame);
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
		case WM_DROPFILES: {
			TCHAR szFile[sizeof(info.load_rom_file)];
			HDROP hDrop = (HDROP) wParam;
			int i = 0, count = DragQueryFile(hDrop, 0xFFFFFFFF, szFile, sizeof(info.load_rom_file));

			for (i = 0; i < count; i++) {
				DragQueryFile(hDrop, i, szFile, sizeof(info.load_rom_file));
			}

			DragFinish(hDrop);

			if (count) {
				emu_pause(TRUE);
				change_rom(szFile);
				emu_pause(FALSE);
			}

			break;
		}
		case WM_SYSCOMMAND: {
			switch (wParam & 0xFFF0) {
				case SC_MONITORPOWER:
				case SC_SCREENSAVE:
					return (0);
			}
			break;
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDM_FILE_OPEN:
					/*
					 * nella finestra di apertura file posso esserci
					 * arrivato anche premendo CTRL+O, quindi in uscita
					 * potrei ancora avere il tl.key settato.
					 */
					tl.key = FALSE;
					open_event();
					SetFocus(sdl_frame);
					break;
				case IDM_FILE_RECENT_0:
				case IDM_FILE_RECENT_1:
				case IDM_FILE_RECENT_2:
				case IDM_FILE_RECENT_3:
				case IDM_FILE_RECENT_4:
				case IDM_FILE_RECENT_5:
				case IDM_FILE_RECENT_6:
				case IDM_FILE_RECENT_7:
				case IDM_FILE_RECENT_8:
				case IDM_FILE_RECENT_9:
				case IDM_FILE_RECENT_10:
				case IDM_FILE_RECENT_11:
				case IDM_FILE_RECENT_12:
				case IDM_FILE_RECENT_13:
				case IDM_FILE_RECENT_14: {
					int index = LOWORD(wParam) - IDM_FILE_RECENT_0;

					emu_pause(TRUE);

					if (strncmp(recent_roms_list.current, recent_roms_list.item[index],
					        RECENT_ROMS_LINE) != 0) {
						change_rom(recent_roms_list.item[index]);
					}
					emu_pause(FALSE);
					break;
				}
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
					save_slot_action(SAVE);
					break;
				case IDM_SET_SAVE_LOAD:
					save_slot_action(LOAD);
					break;
				case IDM_SET_SAVE_INC:
					save_slot_incdec(INC);
					break;
				case IDM_SET_SAVE_DEC:
					save_slot_incdec(DEC);
					break;
				case IDM_SET_SAVE_0:
					save_slot_set(0);
					break;
				case IDM_SET_SAVE_1:
					save_slot_set(1);
					break;
				case IDM_SET_SAVE_2:
					save_slot_set(2);
					break;
				case IDM_SET_SAVE_3:
					save_slot_set(3);
					break;
				case IDM_SET_SAVE_4:
					save_slot_set(4);
					break;
				case IDM_SET_SAVE_5:
					save_slot_set(5);
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
					set_fps(FPS_DEFAULT);
					break;
				case IDM_SET_FPS_60:
					set_fps(FPS_60);
					break;
				case IDM_SET_FPS_59:
					set_fps(FPS_59);
					break;
				case IDM_SET_FPS_58:
					set_fps(FPS_58);
					break;
				case IDM_SET_FPS_57:
					set_fps(FPS_57);
					break;
				case IDM_SET_FPS_56:
					set_fps(FPS_56);
					break;
				case IDM_SET_FPS_55:
					set_fps(FPS_55);
					break;
				case IDM_SET_FPS_54:
					set_fps(FPS_54);
					break;
				case IDM_SET_FPS_53:
					set_fps(FPS_53);
					break;
				case IDM_SET_FPS_52:
					set_fps(FPS_52);
					break;
				case IDM_SET_FPS_51:
					set_fps(FPS_51);
					break;
				case IDM_SET_FPS_50:
					set_fps(FPS_50);
					break;
				case IDM_SET_FPS_49:
					set_fps(FPS_49);
					break;
				case IDM_SET_FPS_48:
					set_fps(FPS_48);
					break;
				case IDM_SET_FPS_47:
					set_fps(FPS_47);
					break;
				case IDM_SET_FPS_46:
					set_fps(FPS_46);
					break;
				case IDM_SET_FPS_45:
					set_fps(FPS_45);
					break;
				case IDM_SET_FPS_44:
					set_fps(FPS_44);
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
					set_overscan(OSCAN_ON);
					break;
				case IDM_SET_OSCAN_OFF:
					set_overscan(OSCAN_OFF);
					break;
				case IDM_SET_OSCAN_DEF:
					set_overscan(OSCAN_DEFAULT);
					break;
				case IDM_SET_OSCAN_DEFAULT_ON:
					set_overscan(OSCAN_DEFAULT_ON);
					break;
				case IDM_SET_OSCAN_DEFAULT_OFF:
					set_overscan(OSCAN_DEFAULT_OFF);
					break;
				case IDM_SET_FILTER_NO_FILTER:
					set_filter(NO_FILTER);
					break;
				case IDM_SET_FILTER_BILINEAR:
					set_filter(BILINEAR);
					break;
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
					set_filter(CRT_CURVE);
					break;
				case IDM_SET_FILTER_CRTNOCURVE:
					set_filter(CRT_NO_CURVE);
					break;
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
					cfg->ntsc_format = COMPOSITE;
					set_filter(NTSC_FILTER);
					break;
				case IDM_SET_FILTER_RGBNTSCSVD:
					cfg->ntsc_format = SVIDEO;
					set_filter(NTSC_FILTER);
					break;
				case IDM_SET_FILTER_RGBNTSCRGB:
					cfg->ntsc_format = RGBMODE;
					set_filter(NTSC_FILTER);
					break;
				case IDM_SET_PALETTE_PAL:
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_PAL, FALSE);
					break;
				case IDM_SET_PALETTE_NTSC:
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_NTSC, FALSE);
					break;
				case IDM_SET_PALETTE_SONY:
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_SONY, FALSE);
					break;
				case IDM_SET_PALETTE_MONO:
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_MONO, FALSE);
					break;
				case IDM_SET_PALETTE_GREEN:
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_GREEN, FALSE);
					break;
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
					gui_fullscreen();
					break;
				case IDM_SET_STRETCHFLSCR:
					cfg->aspect_ratio = !cfg->aspect_ratio;
					if (cfg->fullscreen == FULLSCR) {
						gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
					}
					gui_update();
					break;
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
				case IDM_SET_STEREO_DELAY_5:
				case IDM_SET_STEREO_DELAY_10:
				case IDM_SET_STEREO_DELAY_15:
				case IDM_SET_STEREO_DELAY_20:
				case IDM_SET_STEREO_DELAY_25:
				case IDM_SET_STEREO_DELAY_30:
				case IDM_SET_STEREO_DELAY_35:
				case IDM_SET_STEREO_DELAY_40:
				case IDM_SET_STEREO_DELAY_45:
				case IDM_SET_STEREO_DELAY_50:
				case IDM_SET_STEREO_DELAY_55:
				case IDM_SET_STEREO_DELAY_60:
				case IDM_SET_STEREO_DELAY_65:
				case IDM_SET_STEREO_DELAY_70:
				case IDM_SET_STEREO_DELAY_75:
				case IDM_SET_STEREO_DELAY_80:
				case IDM_SET_STEREO_DELAY_85:
				case IDM_SET_STEREO_DELAY_90:
				case IDM_SET_STEREO_DELAY_95:
				case IDM_SET_STEREO_DELAY_100:
					set_stereo_delay(((LOWORD(wParam) - IDM_SET_STEREO_DELAY_5) + 1) * 5);
					break;
				case IDM_SET_AUDIO_SWAP_DUTY:
					emu_pause(TRUE);
					cfg->swap_duty = !cfg->swap_duty;
					gui_update();
					emu_pause(FALSE);
					break;
				case IDM_SET_AUDIO_QUALITY_LOW:
					set_audio_quality(AQ_LOW);
					break;
				case IDM_SET_AUDIO_QUALITY_HIGH:
					set_audio_quality(AQ_HIGH);
					break;
				case IDM_SET_AUDIO_APU_CHANNELS:
					apu_channels_dialog(hwnd);
					break;
				case IDM_SET_AUDIO_ENABLE:
					emu_pause(TRUE);
					cfg->apu.channel[APU_MASTER] = !cfg->apu.channel[APU_MASTER];
					if (cfg->apu.channel[APU_MASTER]) {
						snd_start();
					} else {
						snd_stop();
					}
					gui_update();
					emu_pause(FALSE);
					break;
				case IDM_SET_GAMEGENIE:
					set_gamegenie();
					break;
				case IDM_SET_SAVENOW:
					cfg_file_save();
					break;
				case IDM_SET_SAVEONEXIT:
					cfg->save_on_exit = !cfg->save_on_exit;
					gui_update();
					break;
				case IDM_HELP_ABOUT:
					if (!info.portable) {
						DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd,
						        (DLGPROC) about_proc);
					} else {
						DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT_PORTABLE), hwnd,
						        (DLGPROC) about_proc);
					}
					SetFocus(sdl_frame);
					break;
				case IDM_SET_INPUT_CONFIG:
					cfg_input_dialog(hwnd);
					break;
			}
			break;
		}
		case WM_PAINT:
			time_handler_redraw();
			ValidateRect(hwnd, NULL);
			return (0);
		case WM_CLOSE:
			/*
			 * blocco l'audio gia' qui perche' l'engine XAudio2
			 * necessita che la gestione degli eventi gui_event()
			 * funzioni ancora per potersi bloccare correttamente.
			 * Impostando l'info.stop a TRUE il gui_event() non verrebbe
			 * piu' chiamato e l'emulatore crasharebbe (almeno questo
			 * accade da Vista in poi e non su XP).
			 */
			snd_stop();
			info.stop = TRUE;
			break;
		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS);
			return (1);
		case WM_ERASEBKGND:
			return (1);
		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
long __stdcall timeline_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	BYTE dec = 0, snap = SendMessage(hTimeline, TBM_GETPOS, 0, 0);
	LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT) lParam;
	LPNMCUSTOMDRAW lpDraw;
	static char szBuf[80] = "";

	if (!tl.button && (GetForegroundWindow() == main_win)) {
		SetFocus(sdl_frame);
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
						tl_down(tl.button);
					}
					break;
				case TTN_POP:
					tl_up(tl.button);
					break;
				case TTN_NEEDTEXT:
					if (tl.snaps_fill) {
						dec = ((tl.snaps_fill - 1) - snap) * TL_SNAP_SEC;
					}
					if (!dec) {
						sprintf(szBuf, "%d sec", 0);
					} else {
						sprintf(szBuf, "% 2d sec", -abs(((tl.snaps_fill - 1) - snap) * TL_SNAP_SEC));
					}
					lpToolTipText->lpszText = szBuf;
					break;
			}
			break;
		}
		case WM_HSCROLL:
			wrap_tl_preview(snap);
			return (FALSE);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
long __stdcall save_slot_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DRAWITEMSTRUCT *pdis;
	MEASUREITEMSTRUCT *pmis;

	switch (msg) {
		case WM_MEASUREITEM: {
			pmis = (MEASUREITEMSTRUCT *) lParam;
			switch (pmis->CtlID) {
				case ID_SAVE_SLOT_CB: {
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
				case ID_SAVE_SLOT_CB: {
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
						save_slot_preview(slot);
					} else {
						back_colour = RGB(255, 255, 255);
						back_brush = CreateSolidBrush(back_colour);
						FillRect(pdis->hDC, &pdis->rcItem, back_brush);
						DeleteObject(back_brush);
						SetBkColor(pdis->hDC, back_colour);
						if (save_slot.state[slot]) {
							SetTextColor(pdis->hDC, RGB(0, 0, 0));
						} else {
							SetTextColor(pdis->hDC, RGB(200, 200, 200));
						}
					}
					DrawText(pdis->hDC, string, strlen(string), &pdis->rcItem,
					        DT_LEFT | DT_SINGLELINE);
					break;
				}
				case ID_SAVE_SLOT_BL:
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
				case ID_SAVE_SLOT_BS:
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
			switch (LOWORD(wParam)) {
				case ID_SAVE_SLOT_BS: {
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							save_slot_action(SAVE);
							break;
					}
					SetFocus(sdl_frame);
					break;
				}
				case ID_SAVE_SLOT_BL: {
					switch (HIWORD(wParam)) {
						case BN_CLICKED:
							save_slot_action(LOAD);
							break;
					}
					SetFocus(sdl_frame);
					break;
				}
				case ID_SAVE_SLOT_CB: {
					switch (HIWORD(wParam)) {
						case CBN_DROPDOWN:
							emu_pause(TRUE);
							break;
						case CBN_CLOSEUP:
							save_slot.slot = SendMessage(hSaveslot, CB_GETCURSEL, 0, 0);
							gui_update();
							save_slot.preview_start = FALSE;
							emu_pause(FALSE);
							SetFocus(sdl_frame);
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
long __stdcall about_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY:
			DeleteObject(about_img);
			DeleteObject(about_mask);
			PostQuitMessage(0);
			break;
		case WM_PAINT: {
			BITMAP bm;
			PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hwnd, &ps);

			HDC hdc_mem = CreateCompatibleDC(hdc);
			HBITMAP hbm_old = SelectObject(hdc_mem, about_img);

			GetObject(about_img, sizeof(bm), &bm);

			SelectObject(hdc_mem, about_mask);
			BitBlt(hdc, 90, 30, bm.bmWidth, bm.bmHeight, hdc_mem, 0, 0, SRCAND);

			SelectObject(hdc_mem, about_img);
			BitBlt(hdc, 90, 30, bm.bmWidth, bm.bmHeight, hdc_mem, 0, 0, SRCPAINT);

			SelectObject(hdc_mem, hbm_old);
			DeleteDC(hdc_mem);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_INITDIALOG:
			about_img = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ABOUT));
			about_mask = create_bitmap_mask(about_img, RGB(255, 255, 255));
			return (TRUE);
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
			}
			break;
		}
		default:
			return (FALSE);
	}
	return (TRUE);
}
double high_resolution_ms(void) {
	uint64_t time, diff;

	QueryPerformanceCounter((LARGE_INTEGER *) &time);
	diff = ((time - gui.counter_start) * 1000) / gui.frequency;

	return ((double) (diff & 0xffffffff));
}
void open_event(void) {
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[1024];      // buffer for file name

	emu_pause(TRUE);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = main_win;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All supported formats\0*.nes;*.NES;*.fds;*.FDS;*.fm2;*.FM2\0"
			"Nes rom files\0*.nes;*.NES\0"
			"FDS image files\0*.fds;*.FDS\0"
			"TAS movie files\0*.fm2;*.FM2\0"
			"All files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (cfg->fullscreen == FULLSCR) {
		/* nascondo la finestra */
		ShowWindow(main_win, SW_HIDE);
	}

	// Display the Open dialog box.
	if (GetOpenFileName(&ofn) == TRUE) {
		change_rom(ofn.lpstrFile);
	}

	if (cfg->fullscreen == FULLSCR) {
		/* visualizzo la finestra */
		ShowWindow(main_win, SW_NORMAL);
		/* setto il focus*/
		SetFocus(sdl_frame);
	}

	emu_pause(FALSE);
}
void change_menuitem(BYTE check_or_enab, UINT type, UINT menuitem_id) {
	if (check_or_enab == CHECK) {
		CheckMenuItem(main_menu, menuitem_id, MF_BYCOMMAND | type);
	} else {
		EnableMenuItem(main_menu, menuitem_id, MF_BYCOMMAND | type);
	}
}
void make_reset(BYTE type) {
	if (type == HARD) {
		if (cfg->gamegenie && gamegenie.rom_present) {
			if (info.mapper != GAMEGENIE_MAPPER) {
				strcpy(info.load_rom_file, info.rom_file);
			}
			gamegenie_reset(TRUE);
			type = CHANGE_ROM;
		} else {
			/*
			 * se e' stato disabilitato il game genie quando ormai
			 * e' gia' in esecuzione e si preme un reset, carico la rom.
			 */
			if (info.mapper == GAMEGENIE_MAPPER) {
				gamegenie_reset(TRUE);
				type = CHANGE_ROM;
			}
		}
	}

	if (emu_reset(type)) {
		PostMessage(main_win, WM_CLOSE, EXIT_FAILURE, 0);
	}
}
void set_mode(BYTE mode) {
	BYTE reset = TRUE;

	if (mode == cfg->mode) {
		return;
	}

	switch (mode) {
		case PAL:
		case NTSC:
		case DENDY:
			if ((cfg->mode == AUTO) && (mode == machine.type)) {
				reset = FALSE;
			}
			cfg->mode = mode;
			machine = machinedb[mode - 1];
			break;
		case AUTO:
			if (info.machine_db == PAL) {
				machine = machinedb[PAL - 1];
			} else if (info.machine_db == DENDY) {
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
			if ((cfg->mode == info.machine_db)
			        || ((cfg->mode == NTSC) && ((info.machine_db < NTSC) || (info.machine_db > DENDY)))) {
				reset = FALSE;
			}
			cfg->mode = AUTO;
			break;
	}

	gui_update();

	if (reset) {
		text_add_line_info(1, "switched to [green]%s", param_fps[machine.type].lname);
		make_reset(CHANGE_MODE);
	}
}
void set_scale(BYTE scale) {
	if (cfg->scale == scale) {
		return;
	}

	ShowWindow(main_win, SW_HIDE);

	switch (scale) {
		case X1:
			gfx_set_screen(X1, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case X2:
			gfx_set_screen(X2, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case X3:
			gfx_set_screen(X3, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case X4:
			gfx_set_screen(X4, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
	}

	ShowWindow(main_win, SW_NORMAL);
}
void set_overscan(BYTE oscan) {
	LockWindowUpdate(main_win);

	switch (oscan) {
		case OSCAN_ON:
		case OSCAN_OFF:
		case OSCAN_DEFAULT:
			cfg->oscan = oscan;
			cfg_file_pgs_save();
			break;
		case OSCAN_DEFAULT_OFF:
			cfg->oscan_default = OSCAN_OFF;
			break;
		case OSCAN_DEFAULT_ON:
			cfg->oscan_default = OSCAN_ON;
			break;
	}

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);

	LockWindowUpdate(NULL);
}
void set_filter(BYTE filter) {
	LockWindowUpdate(main_win);

	switch (filter) {
		case NO_FILTER:
			gfx_set_screen(NO_CHANGE, NO_FILTER, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case BILINEAR:
			gfx_set_screen(NO_CHANGE, BILINEAR, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case POSPHOR:
			gfx_set_screen(NO_CHANGE, POSPHOR, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case SCANLINE:
			gfx_set_screen(NO_CHANGE, SCANLINE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case DBL:
			gfx_set_screen(NO_CHANGE, DBL, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case CRT_CURVE:
			gfx_set_screen(NO_CHANGE, CRT_CURVE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case CRT_NO_CURVE:
			gfx_set_screen(NO_CHANGE, CRT_NO_CURVE, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case SCALE2X:
			gfx_set_screen(X2, SCALE2X, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case SCALE3X:
			gfx_set_screen(X3, SCALE3X, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case SCALE4X:
			gfx_set_screen(X4, SCALE4X, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case HQ2X:
			gfx_set_screen(X2, HQ2X, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case HQ3X:
			gfx_set_screen(X3, HQ3X, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case HQ4X:
			gfx_set_screen(X4, HQ4X, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case NTSC_FILTER:
			gfx_set_screen(NO_CHANGE, NTSC_FILTER, NO_CHANGE, NO_CHANGE, FALSE);
			if (cfg->filter == NTSC_FILTER) {
				ntsc_set(cfg->ntsc_format, 0, 0, (BYTE *) palette_RGB, 0);
				gui_update();
			}
			break;
	}

	LockWindowUpdate(NULL);
}
void set_rendering(BYTE rendering) {
	if (cfg->render == rendering) {
		return;
	}

	ShowWindow(main_win, SW_HIDE);

	/* switch opengl/software render */
	gfx_set_render(rendering);
	cfg->render = rendering;

	gfx_reset_video();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);

	ShowWindow(main_win, SW_NORMAL);
}
void set_vsync(BYTE vsync) {
	if (cfg->vsync == vsync) {
		return;
	}

	/*
	 * se non nascondo la finestra, al momento del
	 * SDL_QuitSubSystem e del SDL_InitSubSystem
	 * l'applicazione crasha.
	 */
	ShowWindow(main_win, SW_HIDE);

	/* switch vsync */
	cfg->vsync = vsync;

	gfx_reset_video();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);

	ShowWindow(main_win, SW_NORMAL);
}
void set_effect(void) {
	if (input_zapper_is_connected((_port *) &port) == TRUE) {
		return;
	}

	opengl_unset_effect();

	opengl.rotation = !opengl.rotation;

	if (opengl.rotation) {
		opengl_init_effect = opengl_init_cube3d;
		opengl_set_effect = opengl_set_cube3d;
		opengl_unset_effect = opengl_unset_cube3d;
		opengl_draw_scene = opengl_draw_scene_cube3d;

		opengl.factor_distance = opengl.x_rotate = opengl.y_rotate = 0;
		if (cfg->fullscreen == FULLSCR) {
			SDL_ShowCursor(SDL_ENABLE);
		}
	} else {
		opengl_init_effect = opengl_init_no_effect;
		opengl_set_effect = opengl_set_no_effect;
		opengl_unset_effect = opengl_unset_no_effect;
		opengl_draw_scene = opengl_draw_scene_no_effect;

		if (cfg->fullscreen == FULLSCR) {
			SDL_ShowCursor(SDL_DISABLE);
		}
	}

	opengl_init_effect();

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
}
void set_samplerate(int samplerate) {
	if (cfg->samplerate == samplerate) {
		return;
	}
	cfg->samplerate = samplerate;
	snd_start();
	gui_update();
}
void set_channels(int channels) {
	if (cfg->channels == channels) {
		return;
	}
	cfg->channels = channels;
	snd_start();
	gui_update();
}
void set_stereo_delay(int stereo_delay) {
	double delay = ((double) stereo_delay) / 100.f;

	if (cfg->stereo_delay == delay) {
		return;
	}

	cfg->stereo_delay = delay;
	snd_stereo_delay();
	gui_update();
}

void set_audio_quality(int quality) {
	if (cfg->audio_quality == quality) {
		return;
	}
	cfg->audio_quality = quality;
	audio_quality(cfg->audio_quality);
	gui_update();
}
void set_fps(int fps) {
	if (cfg->fps == fps) {
		return;
	}
	cfg->fps = fps;
	emu_pause(TRUE);
	fps_init();
	snd_start();
	gui_update();
	emu_pause(FALSE);
}
void set_frame_skip(int frameskip) {
	if (cfg->frameskip == frameskip) {
		return;
	}
	cfg->frameskip = frameskip;
	if (!fps.fast_forward) {
		fps_normalize();
	}
	gui_update();
}
void set_gamegenie(void) {
	cfg->gamegenie = !cfg->gamegenie;

	if (cfg->gamegenie) {
		gamegenie_check_rom_present(TRUE);
	}

	gui_update();
}
void __stdcall time_handler_redraw(void) {
	gfx_draw_screen(TRUE);
}
HBITMAP create_bitmap_mask(HBITMAP hbm_colour, COLORREF cr_transparent) {
	HDC hdc_mem, hdc_mem2;
	HBITMAP hbm_mask;
	BITMAP bm;

	// Create monochrome (1 bit) mask bitmap.
	GetObject(hbm_colour, sizeof(BITMAP), &bm);
	hbm_mask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	// Get some HDCs that are compatible with the display driver
	hdc_mem = CreateCompatibleDC(0);
	hdc_mem2 = CreateCompatibleDC(0);

	if (SelectBitmap(hdc_mem, hbm_colour) == 0) {
		;
	}
	if (SelectBitmap(hdc_mem2, hbm_mask) == 0) {
		;
	}

	// Set the background colour of the colour image to the colour
	// you want to be transparent.
	SetBkColor(hdc_mem, cr_transparent);

	// Copy the bits from the colour image to the B+W mask... everything
	// with the background colour ends up white while everythig else ends up
	// black...Just what we wanted.
	BitBlt(hdc_mem2, 0, 0, bm.bmWidth, bm.bmHeight, hdc_mem, 0, 0, SRCCOPY);

	// Take our new mask and use it to turn the transparent colour in our
	// original colour image to black so the transparency effect will
	// work right.
	BitBlt(hdc_mem, 0, 0, bm.bmWidth, bm.bmHeight, hdc_mem2, 0, 0, SRCINVERT);

	// Clean up.
	DeleteDC(hdc_mem);
	DeleteDC(hdc_mem2);

	return (hbm_mask);
}
void wrap_tl_preview(BYTE snap) {
	if (!(tl.button | tl.key)) {
		return;
	}

	if (!tl.snaps_fill) {
		SendMessage(hTimeline, TBM_SETPOS, TRUE, 0);
		return;
	}

	/* snap non puo' essere mai maggiore del numero di snap effettuate */
	if (snap > (tl.snaps_fill - 1)) {
		SendMessage(hTimeline, TBM_SETPOS, TRUE, tl.snaps_fill - 1);
		snap = (tl.snaps_fill - 1);
	}

	SendMessage(hTimeline, TBM_SETPOS, TRUE, snap);

	if (snap == (tl.snaps_fill - 1)) {
		memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
		time_handler_redraw();
		return;
	}

	timeline_preview(snap);
}
void save_slot_incdec(BYTE mode) {
	BYTE new_slot;

	if (mode == INC) {
		new_slot = save_slot.slot + 1;
		if (new_slot >= SAVE_SLOTS) {
			new_slot = 0;
		}
	} else {
		new_slot = save_slot.slot - 1;
		if (new_slot >= SAVE_SLOTS) {
			new_slot = SAVE_SLOTS - 1;
		}
	}
	save_slot_set(new_slot);
}
void save_slot_action(BYTE mode) {
	emu_pause(TRUE);

	if (mode == SAVE) {
		save_slot_save();
		cfg_file_pgs_save();
	} else {
		save_slot_load();
	}

	gui_update();

	emu_pause(FALSE);
}
void save_slot_set(BYTE selection) {
	SendMessage(hSaveslot, CB_SETCURSEL, selection, 0);
	save_slot.slot = selection;
	gui_update();
}
void fds_eject_insert_disk(void) {
	if (!fds.drive.disk_ejected) {
		fds_disk_op(FDS_DISK_EJECT, 0);
	} else {
		fds_disk_op(FDS_DISK_INSERT, 0);
	}

	gui_update();
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

	gui_update();
}
void change_rom(char *rom) {
	strcpy(info.load_rom_file, rom);
	/*
	 * nascondo la finestra perche' la nuova rom potrebbe
	 * avere una configurazione dell'overscan diversa da quella
	 * della rom precedente e quindi potrei essere costretto
	 * a fare un SDL_SetVideoMode con dimensioni x ed y diverse
	 * che sotto windows, se la finestra non e' nascosta, crasha
	 * l'emulatore.
	 */
	ShowWindow(main_win, SW_HIDE);
	//LockWindowUpdate(main_win);

	gamegenie_reset(FALSE);

	make_reset(CHANGE_ROM);

	/* visualizzo nuovamente la finestra */
	ShowWindow(main_win, SW_NORMAL);
	//LockWindowUpdate(NULL);
	gui_update();
}
