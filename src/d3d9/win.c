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
#include "clock.h"
#include "version.h"
#include "gfx.h"
#include "cfg_file.h"
#include "gamegenie.h"
#include "text.h"

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

void set_mode(BYTE mode);
void set_scale(BYTE scale);
void set_filter(BYTE filter);

void open_event(void);
void make_reset(BYTE type);
void change_rom(char *rom);

static HWND main_win, d3d_frame;
HMENU main_menu;
HACCEL acc_keys;

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
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
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

	main_menu = GetMenu(main_win);

	acc_keys = LoadAccelerators(gui.main_hinstance, "IDR_ACCKEYS");

	/* ---------------------------------- D3D frame ---------------------------------- */

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
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDM_FILE_OPEN:
					/*
					 * nella finestra di apertura file posso esserci
					 * arrivato anche premendo CTRL+O, quindi in uscita
					 * potrei ancora avere il tl.key settato.
					 */
					//tl.key = FALSE;
					open_event();
					SetFocus(d3d_frame);
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
				/*
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
				*/
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
				/*
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
				*/
				case IDM_SET_SIZE_1X:
					set_scale(X1);
					break;
				case IDM_SET_SIZE_2X:
					set_scale(X2);
					break;
				case IDM_SET_SIZE_3X:
					set_scale(X3);
					break;
				case IDM_SET_SIZE_4X:
					set_scale(X4);
					break;
				/*
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
				*/
				case IDM_SET_FILTER_NO_FILTER:
					set_filter(NO_FILTER);
					break;
				case IDM_SET_FILTER_BILINEAR:
					set_filter(BILINEAR);
					break;
				/*
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
				*/
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
				/*
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
				case IDM_SET_AUDIO_ENABLE:
					emu_pause(TRUE);
					cfg->audio = !cfg->audio;
					if (cfg->audio) {
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
					cfg_input(hwnd);
					break;
			*/
			}
			break;
		}
		case WM_DROPFILES: {
			TCHAR file[sizeof(info.load_rom_file)];
			HDROP drop = (HDROP) wParam;

			int i = 0, count = DragQueryFile(drop, 0xFFFFFFFF, file, sizeof(info.load_rom_file));

			for (i = 0; i < count; i++) {
				DragQueryFile(drop, i, file, sizeof(info.load_rom_file));
			}

			DragFinish(drop);

			if (count) {
				emu_pause(TRUE);
				change_rom(file);
				emu_pause(FALSE);
			}

			break;
		}
		case WM_CLOSE:
			info.stop = TRUE;
			break;
		case WM_DESTROY:
			PostQuitMessage(EXIT_SUCCESS);
			return (1);
		default:
			break;
	}

	return (DefWindowProc(hwnd, msg, wParam, lParam));
}
double high_resolution_ms(void) {
	uint64_t time, diff;

	QueryPerformanceCounter((LARGE_INTEGER *) &time);
	diff = ((time - gui.counter_start) * 1000) / gui.frequency;

	return ((double) (diff & 0xffffffff));
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
			if ((cfg->mode == info.machine_db) || ((cfg->mode == NTSC) &&
					((info.machine_db < NTSC) || (info.machine_db > DENDY)))) {
				reset = FALSE;
			}
			cfg->mode = AUTO;
			break;
	}

	gui_update();

	if (reset) {
		//text_add_line_info(1, "switched to [green]%s", param_fps[machine.type].lname);
		make_reset(CHANGE_MODE);
	}
}
void set_scale(BYTE scale) {
	if (cfg->scale == scale) {
		return;
	}

	//ShowWindow(main_win, SW_HIDE);

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

	//ShowWindow(main_win, SW_NORMAL);
}
void set_filter(BYTE filter) {
	//LockWindowUpdate(main_win);

	switch (filter) {
		case NO_FILTER:
			gfx_set_screen(NO_CHANGE, NO_FILTER, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		case BILINEAR:
			gfx_set_screen(NO_CHANGE, BILINEAR, NO_CHANGE, NO_CHANGE, FALSE);
			break;
		/*
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
		*/
		case NTSC_FILTER:
			gfx_set_screen(NO_CHANGE, NTSC_FILTER, NO_CHANGE, NO_CHANGE, FALSE);
			if (cfg->filter == NTSC_FILTER) {
				ntsc_set(cfg->ntsc_format, 0, 0, (BYTE *) palette_RGB, 0);
				gui_update();
			}
			break;
	}

	//LockWindowUpdate(NULL);
}

void open_event(void) {
	OPENFILENAME ofn;
	char file[1024];

	emu_pause(TRUE);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = main_win;
	ofn.lpstrFile = file;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(file);
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

	if (GetOpenFileName(&ofn) == TRUE) {
		change_rom(ofn.lpstrFile);
	}

	emu_pause(FALSE);
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
	//ShowWindow(main_win, SW_HIDE);
	//LockWindowUpdate(main_win);

	gamegenie_reset(FALSE);

	make_reset(CHANGE_ROM);

	/* visualizzo nuovamente la finestra */
	//ShowWindow(main_win, SW_NORMAL);
	//LockWindowUpdate(NULL);

	gui_update();
}
