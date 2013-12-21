/*
 * cfg_std_pad.c
 *
 *  Created on: 27/nov/2013
 *      Author: fhorse
 */

#if defined MINGW32
#define _WIN32_IE 0x0300
#endif
#include "gui.h"
#include "dinput.h"
#include "dxerr8.h"
#include "commctrl.h"
#include "richedit.h"
#include "cfg_std_pad.h"

long __stdcall cfg_std_pad_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_std_pad_kbd_joy_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_std_pad_kbd_read(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_std_pad_joy_esc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __stdcall cfg_std_pad_joy_read(void);

DWORD WINAPI cfg_std_pad_in_sequence(LPVOID lpParam);

void cfg_std_pad_info_entry_print(const char *fmt, ...);

void cfg_std_pad_tab_init(void);
void cfg_std_pad_enable_page_and_other(int no_disable, int mode);
void cfg_std_pad_enable_page_buttons(int mode);
void cfg_std_pad_unset(HWND hwnd, int button);
void cfg_std_pad_joy_timer_kill(void);

void cfg_std_pad_destroy(HWND hwnd, INT_PTR nResult);
BOOL CALLBACK di8_enum_peripherals(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

#define PVBUTTON cfg_std_pad.vbutton
#define PTYPE    cfg_std_pad.type
#define read_axis_joy(axis, info)\
	value = (axis << 1) + 1;\
	if (joy_info.info > CENTER) {\
		value++;\
	}

enum std_pad_combo_value { MAX_JOYSTICK = 16 };

typedef struct {
	BYTE present;
	BYTE line;
	BYTE id;
	char description[100];
} _combo_element;

struct _cfg_std_pad {
	HWND page;
	HWND pressed;
	WNDPROC old_wnd_proc;
	HANDLE thread;

	BYTE joy_count;
	BYTE type;
	BYTE vbutton;
	BYTE no_other_buttons;
	BYTE joy_timer_run;

	BYTE in_sequence;
	BYTE exit_thread;

	_combo_element joy[MAX_JOYSTICK + 1];

	_cfg_port cfg;
	_cfg_port *cfg_port_father;
} cfg_std_pad;

void cfg_std_pad_dialog(HWND hwnd, _cfg_port *cfg_port) {
	memset(&cfg_std_pad, 0x00, sizeof(cfg_std_pad));
	memcpy(&cfg_std_pad.cfg, cfg_port, sizeof(_cfg_port));

	/* disabilito gli acceleratori */
	gui.accelerators_anabled = FALSE;

	cfg_std_pad.cfg_port_father = cfg_port;

	cfg_input.child = CreateDialog(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_STANDARD_PAD_DIALOG), hwnd, (DLGPROC) cfg_std_pad_messages);

	cfg_std_pad.thread = CreateThread( NULL, 0, cfg_std_pad_in_sequence, NULL, 0, NULL);

	gui_sleep(30);

	{
		char label[50];

		snprintf(label, sizeof(label), "Controller %d : Standard Pad", cfg_port->id);

		SetDlgItemText(cfg_input.child, IDC_STANDARD_PAD_CONTROL_BOX, label);
	}
}

long __stdcall cfg_std_pad_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			cfg_std_pad_destroy(hwnd, 0);
			return (TRUE);
		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->code) {
				case TCN_SELCHANGE: {
					HWND tab = GetDlgItem(hwnd, IDC_STANDARD_PAD_TAB);

					if (cfg_std_pad.no_other_buttons == TRUE) {
						(void) TabCtrl_SetCurSel(tab, PTYPE);
					} else {
						cfg_std_pad.type = TabCtrl_GetCurSel(tab);
						cfg_std_pad_tab_init();
					}
					return (TRUE);
				}
			}
			break;
		case WM_INITDIALOG: {
			/*
			{
				cfg_std_pad.img = LoadBitmap(GetModuleHandle(NULL),
				        MAKEINTRESOURCE(IDB_NES_STD_PAD));
				if (cfg_std_pad.img != NULL) {
					SendDlgItemMessage(hwnd, IDC_STANDARD_PAD_IMAGE, STM_SETIMAGE,
					        (WPARAM) IMAGE_BITMAP, (LPARAM) cfg_std_pad.img);
				}
			}
			*/

			{
				HWND tab;
				TCITEM tcitem;

				tab = GetDlgItem(hwnd, IDC_STANDARD_PAD_TAB);

				tcitem.mask = TCIF_TEXT;

				tcitem.pszText = (LPSTR) "Keyboard";
				(void) TabCtrl_InsertItem(tab, 0, &tcitem);

				tcitem.pszText = (LPSTR) "Joystick";
				(void) TabCtrl_InsertItem(tab, 1, &tcitem);

				cfg_std_pad.type = KEYBOARD;
				(void) TabCtrl_SetCurSel(tab, PTYPE);

				cfg_std_pad.page = CreateDialog(GetModuleHandle(NULL),
				        MAKEINTRESOURCE(IDD_KBD_JOY_TAB_PAGE), tab,
				        (DLGPROC) cfg_std_pad_kbd_joy_messages);

				cfg_std_pad_tab_init();
			}
			{
				int i;

				for (i = IDC_TURBO_DELAY_A_SLIDER; i <= IDC_TURBO_DELAY_B_SLIDER; i++) {
					int pos = cfg_std_pad.cfg.port.turbo[i - IDC_TURBO_DELAY_A_SLIDER].frequency;

					SendDlgItemMessage(hwnd, i, TBM_SETTIPSIDE, (WPARAM) TBTS_TOP, 0);
					SendDlgItemMessage(hwnd, i, TBM_SETRANGE, (WPARAM) TRUE,
					        (LPARAM) MAKELONG(1, TURBO_BUTTON_DELAY_MAX));
					SendDlgItemMessage(hwnd, i, TBM_SETTIC, 0, (LPARAM) TURBO_BUTTON_DELAY_DEFAULT);
					SendDlgItemMessage(hwnd, i, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) pos);
				}
			}

			ShowWindow(cfg_input.father, SW_HIDE);

			return (TRUE);
		}
		case WM_HSCROLL: {
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			int type = ctrlID - IDC_TURBO_DELAY_A_SLIDER;
			int value = SendDlgItemMessage(hwnd, ctrlID, TBM_GETPOS, 0, 0);

			cfg_std_pad.cfg.port.turbo[type].frequency = value;
			cfg_std_pad.cfg.port.turbo[type].counter = 0;

			return (TRUE);
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					cfg_std_pad_destroy(hwnd, IDOK);

					/* la mamcpy deve andare necessariamente dopo il destroy */
					memcpy(cfg_std_pad.cfg_port_father, &cfg_std_pad.cfg, sizeof(_cfg_port));
					break;
				case IDCANCEL:
					cfg_std_pad_destroy(hwnd, IDCANCEL);
					break;
			}
			return (TRUE);
	}
	return (FALSE);
}
long __stdcall cfg_std_pad_kbd_joy_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			cfg_std_pad_destroy(hwnd, 0);
			return (TRUE);
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_DEVICE_COMBO:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						BYTE index = SendDlgItemMessage(hwnd, IDC_DEVICE_COMBO, CB_GETCURSEL, 0, 0);
						BYTE i;

						for (i = 0; i < MAX_JOYSTICK + 1; i++) {
							_combo_element *ele = &cfg_std_pad.joy[i];

							if (ele->line == index) {
								cfg_std_pad.cfg.port.joy_id = ele->id;

								if (ele->id == name_to_jsn("NULL")) {
									cfg_std_pad_enable_page_buttons(FALSE);
								} else {
									cfg_std_pad_enable_page_buttons(TRUE);
								}

								break;
							}
						}
						break;
					}
					return (TRUE);
				case IDC_A_BUTTON:
				case IDC_B_BUTTON:
				case IDC_SELECT_BUTTON:
				case IDC_START_BUTTON:
				case IDC_UP_BUTTON:
				case IDC_DOWN_BUTTON:
				case IDC_LEFT_BUTTON:
				case IDC_RIGHT_BUTTON:
				case IDC_TA_BUTTON:
				case IDC_TB_BUTTON: {
					int bt = LOWORD(wParam);
					char *name;

					if (cfg_std_pad.no_other_buttons) {
						SetFocus(cfg_std_pad.pressed);
						return (TRUE);
					}

					cfg_std_pad.vbutton = bt - IDC_A_BUTTON;
					cfg_std_pad.pressed = GetDlgItem(cfg_std_pad.page, bt);
					cfg_std_pad.no_other_buttons = TRUE;

					cfg_std_pad_enable_page_and_other(bt, FALSE);

					SendMessage(cfg_std_pad.pressed, WM_SETTEXT, 0, (LPARAM) "...");

					if (PTYPE == KEYBOARD) {
						name = keyval_to_name(cfg_std_pad.cfg.port.input[PTYPE][PVBUTTON]);

						cfg_std_pad.old_wnd_proc = (WNDPROC) SetWindowLongPtr(cfg_std_pad.pressed,
						        GWLP_WNDPROC, (LONG_PTR) cfg_std_pad_kbd_read);
					} else {
						cfg_std_pad.joy_timer_run = TRUE;

						name = jsv_to_name(cfg_std_pad.cfg.port.input[PTYPE][PVBUTTON]);

						cfg_std_pad.old_wnd_proc = (WNDPROC) SetWindowLongPtr(cfg_std_pad.pressed,
						        GWLP_WNDPROC, (LONG_PTR) cfg_std_pad_joy_esc);

						SetTimer(hwnd, IDT_TIMER2, 150, (TIMERPROC) cfg_std_pad_joy_read);
					}

					cfg_std_pad_info_entry_print("Press a key (ESC for the previous value \"%s\")",
							name);

					SetFocus(cfg_std_pad.pressed);

					return (TRUE);
				}
				case IDC_A_UNSET_BUTTON:
				case IDC_B_UNSET_BUTTON:
				case IDC_SELECT_UNSET_BUTTON:
				case IDC_START_UNSET_BUTTON:
				case IDC_UP_UNSET_BUTTON:
				case IDC_DOWN_UNSET_BUTTON:
				case IDC_LEFT_UNSET_BUTTON:
				case IDC_RIGHT_USET_BUTTON:
				case IDC_TA_UNSET_BUTTON:
				case IDC_TB_UNSET_BUTTON:
					cfg_std_pad_unset(hwnd, LOWORD(wParam) - IDC_A_UNSET_BUTTON);
					return (TRUE);
				case FHCOMMAND:
					cfg_std_pad_enable_page_and_other(0, TRUE);
					return (TRUE);
				case IDC_IN_SEQUENCE_BUTTON:
					cfg_std_pad.in_sequence = TRUE;
					return (TRUE);
				case IDC_UNSET_ALL_BUTTON: {
					BYTE a;

					for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
						cfg_std_pad_unset(hwnd, a);
					}
					return (TRUE);
				}
				case IDC_DEFAULT_BUTTON: {
					BYTE a;

					cfg_std_pad_info_entry_print("");

					cfg_file_set_kbd_joy_default(&cfg_std_pad.cfg.port, cfg_std_pad.cfg.id - 1,
					        PTYPE);

					for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
						char text[30];

						if (PTYPE == KEYBOARD) {
							snprintf(text, sizeof(text), "%s",
							        keyval_to_name(cfg_std_pad.cfg.port.input[PTYPE][a]));
						} else {
							snprintf(text, sizeof(text), "%s",
							        jsv_to_name(cfg_std_pad.cfg.port.input[PTYPE][a]));
						}

						SetDlgItemText(hwnd, a + IDC_A_BUTTON, text);
					}
					return (TRUE);
				}
			}
			break;
	}
	return (FALSE);
}
long __stdcall cfg_std_pad_kbd_read(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN: {
			int keyval = LOWORD(wParam);

			if (keyval == VK_ESCAPE) {
				SendMessage(cfg_std_pad.pressed, WM_SETTEXT, 0,
				        (LPARAM) keyval_to_name(cfg_std_pad.cfg.port.input[PTYPE][PVBUTTON]));
			} else {
				cfg_std_pad.cfg.port.input[PTYPE][PVBUTTON] = keyval;
				SendMessage(cfg_std_pad.pressed, WM_SETTEXT, 0, (LPARAM) keyval_to_name(keyval));
			}

			cfg_std_pad_info_entry_print("");

			cfg_std_pad_enable_page_and_other(0, TRUE);

			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) cfg_std_pad.old_wnd_proc);

			cfg_std_pad.old_wnd_proc = NULL;
			cfg_std_pad.no_other_buttons = FALSE;
			cfg_std_pad.vbutton = 0;

			return (TRUE);
		}
		break;
	}
	return CallWindowProc(cfg_std_pad.old_wnd_proc, hwnd, msg, wParam, lParam);
}
long __stdcall cfg_std_pad_joy_esc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN:
			if (LOWORD(wParam) == VK_ESCAPE) {
				SendMessage(cfg_std_pad.pressed, WM_SETTEXT, 0,
				        (LPARAM) jsv_to_name(cfg_std_pad.cfg.port.input[PTYPE][PVBUTTON]));
				cfg_std_pad_info_entry_print("");
				cfg_std_pad_joy_timer_kill();
			}
			return (TRUE);
	}
	return CallWindowProc(cfg_std_pad.old_wnd_proc, hwnd, msg, wParam, lParam);
}
void __stdcall cfg_std_pad_joy_read(void) {
	JOYINFOEX joy_info;
	JOYCAPS joy_caps;
	WORD value = 0;

	joyGetDevCaps(cfg_std_pad.cfg.port.joy_id, &joy_caps, sizeof(joy_caps));

	joy_info.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_RETURNPOV | JOY_USEDEADZONE;
	joy_info.dwSize = sizeof(joy_info);

	if (joyGetPosEx(cfg_std_pad.cfg.port.joy_id, &joy_info) != JOYERR_NOERROR) {
		return;
	}

	/* esamino i pulsanti */
	if (joy_info.dwButtons) {
		BYTE i;

		for (i = BUT_A; i < MAX_BUTTONS; i++) {
			BYTE button = joy_info.dwButtons & 0x1;

			if (button) {
				value = i | 0x400;
				goto elaborate_value;
			}
			joy_info.dwButtons >>= 1;
		}
	}

	/* esamino gli assi */
	if ((joy_caps.wCaps & JOYCAPS_HASPOV) && (joy_info.dwPOV != JOY_POVCENTERED)) {
		if (joy_info.dwPOV == JOY_POVFORWARD) {
			value = 0x100;
		} else if (joy_info.dwPOV == JOY_POVRIGHT) {
			value = 0x101;
		} else if (joy_info.dwPOV == JOY_POVBACKWARD) {
			value = 0x102;
		} else if (joy_info.dwPOV == JOY_POVLEFT) {
			value = 0x103;
		}
	} else if (joy_info.dwXpos != CENTER) {
		read_axis_joy(X, dwXpos);
	} else if (joy_info.dwYpos != CENTER) {
		read_axis_joy(Y, dwYpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASZ) && (joy_info.dwZpos != CENTER)) {
		read_axis_joy(Z, dwZpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASR) && (joy_info.dwRpos != CENTER)) {
		read_axis_joy(R, dwRpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASU) && (joy_info.dwUpos)) {
		/* FIXME: non so bene come funzionano gli assi U e V */
		//read_axis_joy(U, dwUpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASV) && (joy_info.dwVpos)) {
		//read_axis_joy(V, dwVpos);
	}

	elaborate_value:
	if (value) {
		cfg_std_pad_info_entry_print("");
		cfg_std_pad.cfg.port.input[PTYPE][PVBUTTON] = value;
		SendMessage(cfg_std_pad.pressed, WM_SETTEXT, 0, (LPARAM) jsv_to_name(value));
		cfg_std_pad_joy_timer_kill();
	}
}

DWORD WINAPI cfg_std_pad_in_sequence(LPVOID lpParam) {
	static const int new_order[MAX_STD_PAD_BUTTONS] = {
		UP,	DOWN, LEFT, RIGHT,
		SELECT, START,
		BUT_A, BUT_B, TRB_A, TRB_B,
	};
	BYTE a, brk = FALSE;

	while (TRUE) {
		gui_sleep(30);

		if (cfg_std_pad.exit_thread == TRUE) {
			break;
		}

		if (cfg_std_pad.in_sequence == TRUE) {
			cfg_std_pad_info_entry_print("");

			for (a = BUT_A; a < MAX_STD_PAD_BUTTONS; a++) {
				int bt = IDC_A_BUTTON + new_order[a];

				if (cfg_std_pad.exit_thread == TRUE) {
					brk = TRUE;
					break;
				}

				SendMessage(GetDlgItem(cfg_std_pad.page, bt), BM_CLICK, 0, 0);
				gui_sleep(30);

				while (cfg_std_pad.no_other_buttons == TRUE) {
					gui_sleep(30);
				}
			}
			cfg_std_pad.in_sequence = FALSE;

			if (brk == TRUE) {
				break;
			}

			gui_sleep(30);

			SendMessage(cfg_std_pad.page, WM_COMMAND, LOWORD(FHCOMMAND), 0);
		}
	}

	gui_sleep(30);

	return (0);
}
void cfg_std_pad_info_entry_print(const char *fmt, ...) {
	char buffer[80];
	va_list ap;
	SETTEXTEX st;
	HWND rich = GetDlgItem(cfg_std_pad.page, IDC_INFO_RICHEDIT);

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	st.codepage = CP_ACP;
	st.flags = ST_DEFAULT;
	SendMessage(rich, EM_SETTEXTEX, (WPARAM) &st,(LPARAM) buffer);
}

void cfg_std_pad_tab_init(void) {
	BYTE i;

	cfg_std_pad_info_entry_print("");

	for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		if (PTYPE == KEYBOARD) {
			SetDlgItemText(cfg_std_pad.page, IDC_A_BUTTON + i,
			        keyval_to_name(cfg_std_pad.cfg.port.input[PTYPE][i]));
		} else {
			SetDlgItemText(cfg_std_pad.page, IDC_A_BUTTON + i,
			        jsv_to_name(cfg_std_pad.cfg.port.input[PTYPE][i]));
		}
	}

	SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_RESETCONTENT, 0, 0);

	if (PTYPE == KEYBOARD){
		SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_ADDSTRING, 0,
		        (LPARAM) "Keyboard");

		SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_SETCURSEL, 0, 0);

		EnableWindow(GetDlgItem(cfg_std_pad.page, IDC_DEVICE_COMBO), FALSE);
		cfg_std_pad_enable_page_buttons(TRUE);
	} else {
		BYTE a, disabled_line = 0, count = 0, current_line = name_to_jsn("NULL");
		char description[100];

		{
			LPDIRECTINPUT8 di8 = NULL;
			HRESULT ret;

			cfg_std_pad.joy_count = 0;

			{
				BYTE i;

				for (i = 0; i < MAX_JOYSTICK; i++) {
					memset(&cfg_std_pad.joy[i], 0x00, sizeof(_combo_element));
					cfg_std_pad.joy[i].present = FALSE;
					cfg_std_pad.joy[i].id = 255;
					cfg_std_pad.joy[i].line = 255;
				}
			}

#ifdef __cplusplus
			if ((ret = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
			        IID_IDirectInput8, (LPVOID *) &di8, NULL)) != DI_OK) {
#else
			if ((ret = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
			        &IID_IDirectInput8, (LPVOID *) &di8, NULL)) != DI_OK) {
#endif
				printf("DirectInput8Create error: %s - %s", DXGetErrorString8(ret),
				        DXGetErrorDescription8(ret));
			}

			IDirectInput8_EnumDevices(di8, DI8DEVCLASS_GAMECTRL, di8_enum_peripherals,
			        NULL, DIEDFL_ATTACHEDONLY);

			IDirectInput8_Release(di8);
		}

		for (a = 0; a <= MAX_JOYSTICK; a++) {
			_combo_element *ele = &cfg_std_pad.joy[a];
			BYTE id = a;

			if (a < MAX_JOYSTICK) {
				if (ele->present == FALSE) {
					continue;
				}

				if (id == cfg_std_pad.cfg.port.joy_id) {
					current_line = count;
				}

				snprintf(description, sizeof(description), "js%-2d: %s", id, ele->description);
			} else {
				if (count == 0) {
					break;
				}
				sprintf(description, "Disabled");
				id = name_to_jsn("NULL");
				disabled_line = count;
			}

			SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_ADDSTRING, 0,
			        (LPARAM) description);

			ele->id = id;
			ele->line = count++;
		}

		if (count == 0) {
			SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_ADDSTRING, 0,
			        (LPARAM) "No usable device");
			SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_SETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(cfg_std_pad.page, IDC_DEVICE_COMBO), FALSE);
			cfg_std_pad_enable_page_buttons(FALSE);
		} else {
			EnableWindow(GetDlgItem(cfg_std_pad.page, IDC_DEVICE_COMBO), TRUE);
			if (cfg_std_pad.cfg.port.joy_id == name_to_jsn("NULL")
			        || (current_line == name_to_jsn("NULL"))) {
				SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_SETCURSEL, disabled_line,
				        0);
				cfg_std_pad_enable_page_buttons(FALSE);
			} else {
				SendDlgItemMessage(cfg_std_pad.page, IDC_DEVICE_COMBO, CB_SETCURSEL, current_line,
				        0);
				cfg_std_pad_enable_page_buttons(TRUE);
			}
		}
	}
}

void cfg_std_pad_enable_page_and_other(int no_disable, int mode) {
	if ((mode == TRUE) && (cfg_std_pad.in_sequence == TRUE)) {
		;
	} else {
		int idc;

		cfg_std_pad_enable_page_buttons(mode);

		for (idc = IDC_TURBO_DELAY_A_LABEL_STATIC; idc <= IDC_TURBO_DELAY_B_SLIDER; idc++) {
			EnableWindow(GetDlgItem(cfg_input.child, idc), mode);
		}
	}

	if (mode == FALSE) {
		/* label */
		EnableWindow(GetDlgItem(cfg_std_pad.page, no_disable - MAX_STD_PAD_BUTTONS), TRUE);
		/* button */
		EnableWindow(GetDlgItem(cfg_std_pad.page, no_disable), TRUE);
		/* info */
		EnableWindow(GetDlgItem(cfg_std_pad.page, IDC_INFO_RICHEDIT), TRUE);
	}
}
void cfg_std_pad_enable_page_buttons(int mode) {
	int idc;

	for (idc = IDC_INFO_RICHEDIT; idc <= IDC_TB_UNSET_BUTTON; idc++) {
		EnableWindow(GetDlgItem(cfg_std_pad.page, idc), mode);
	}
}
void cfg_std_pad_unset(HWND hwnd, int button) {
	cfg_std_pad_info_entry_print("");
	cfg_std_pad.cfg.port.input[PTYPE][button] = 0;
	SetDlgItemText(hwnd, button + IDC_A_BUTTON, "NULL");
}

void cfg_std_pad_joy_timer_kill(void) {
	KillTimer(cfg_std_pad.page, IDT_TIMER2);

	cfg_std_pad_enable_page_and_other(0, TRUE);

	SetWindowLongPtr(cfg_std_pad.pressed, GWLP_WNDPROC, (LONG_PTR) cfg_std_pad.old_wnd_proc);

	cfg_std_pad.old_wnd_proc = NULL;
	cfg_std_pad.no_other_buttons = FALSE;
	cfg_std_pad.vbutton = 0;
	cfg_std_pad.joy_timer_run = FALSE;
}

void cfg_std_pad_destroy(HWND hwnd, INT_PTR result) {
	cfg_std_pad.exit_thread = TRUE;
	cfg_std_pad.no_other_buttons = FALSE;
	WaitForSingleObject(cfg_std_pad.thread, INFINITE);
	CloseHandle(cfg_std_pad.thread);

	EndDialog(hwnd, result);
	ShowWindow(cfg_input.father, SW_SHOW);
	cfg_input.child = NULL;

	//DeleteObject(cfg_std_pad.img);

	/* riabilito gli acceleratori */
	gui.accelerators_anabled = TRUE;
}
BOOL CALLBACK di8_enum_peripherals(LPCDIDEVICEINSTANCE lpddi, LPVOID pvref) {
	if (cfg_std_pad.joy_count == MAX_JOYSTICK) {
		return (DIENUM_CONTINUE);
	}

	if (GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_GAMEPAD ||
		GET_DIDEVICE_TYPE(lpddi->dwDevType) == DI8DEVTYPE_JOYSTICK) {
		_combo_element *ele = &cfg_std_pad.joy[cfg_std_pad.joy_count++];

		ele->present = TRUE;
		snprintf(ele->description, sizeof(ele->description), "%s", lpddi->tszProductName);
	}

	return (DIENUM_CONTINUE);
}
