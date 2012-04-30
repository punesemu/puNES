/*
 * cfgstdctrl.c
 *
 *  Created on: 04/nov/2011
 *      Author: fhorse
 */

#include "win.h"
#include "cfgstdctrl.h"
#include "commctrl.h"
#include "param.h"

long __stdcall cfg_standard_controller_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_standard_controller_read_kbd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_standard_controller_joy_esc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __stdcall cfg_standard_controller_read_joy(void);
BOOL cfg_standard_controller_input_is_not_ok(DBWORD input, BYTE type);

#define cfgportinput(type)\
	cfg_std_ctrl.cfg.port.input[type][cfg_std_ctrl.controller_input]
#define read_axis_joy(axis, info)\
	value = (axis << 1) + 1;\
	if (joyInfo.info > CENTER) {\
		value++;\
	}

typedef struct {
	HWND toplevel;
	HWND button_pressed;
	WNDPROC old_wnd_proc;
	BYTE no_other_buttons;
	BYTE controller_input;
	BYTE joy_connected[JOYSTICKID15];
	_cfg_port cfg;
	_cfg_port *cfgport;
} _cfg_standard_controller;

_cfg_standard_controller cfg_std_ctrl;

char cazzata[][15] = {
	"retry",
	"another time",
	"no no no",
	"in use",
	":(",
	"...",
	"ufffff",
	"i don't think"
};

BYTE maxButtons = LENGTH(paramInputP1K);

void cfg_standard_controller(HWND hwnd, _cfg_port *cfgport) {
	char title[30];

	memset(&cfg_std_ctrl, 0, sizeof(cfg_std_ctrl));
	memcpy(&cfg_std_ctrl.cfg, cfgport, sizeof(cfg_std_ctrl.cfg));

	cfg_std_ctrl.cfgport = cfgport;

	cfg_std_ctrl.toplevel = CreateDialog(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_STD_CTRL), hwnd,
			(DLGPROC) cfg_standard_controller_wnd_proc);

	sprintf(title, "Controller %d setup", cfg_std_ctrl.cfg.id);

	SetWindowText(cfg_std_ctrl.toplevel, title);
}
long __stdcall cfg_standard_controller_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DRAWITEMSTRUCT *pdis;

	switch (msg) {
		case WM_INITDIALOG: {
			BYTE i;

			for (i = 0; i < maxButtons; i++) {
				SetDlgItemText(hwnd, IDC_STD_CTRL_KEY_A + i,
				        keyvalToName(cfg_std_ctrl.cfg.port.input[KEYBOARD][i]));
				SetDlgItemText(hwnd, IDC_STD_CTRL_JOY_A + i,
				        jsvToName(cfg_std_ctrl.cfg.port.input[JOYSTICK][i]));
			}

			SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_RESETCONTENT, 0, 0);

			for (i = 0; i < 4; i++) {
				char label[30];

				sprintf(label, "Device %d", i);
				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_ADDSTRING, i, (LPARAM) label);
			}

			if (cfg_std_ctrl.cfg.port.joyID == nameToJsn("NULL")) {
				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_SETCURSEL, 0, 0);
				cfg_std_ctrl.cfg.port.joyID = 0;
			} else {
				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_SETCURSEL,
				        cfg_std_ctrl.cfg.port.joyID, 0);
			}
			return TRUE;
		}
		case WM_DRAWITEM:
			pdis = (DRAWITEMSTRUCT *) lParam;

			if (pdis->CtlID == IDC_STD_CTRL_JOY_ID) {
				JOYINFOEX joyInfo;
				HBRUSH back_brush;
				COLORREF back_colour;
				char string[30];
				int slot = 0;
				BYTE same_port = FALSE;

				if (pdis->itemID == -1) {
					return TRUE;
				}

				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_GETLBTEXT, pdis->itemID,
				        (LPARAM) string);

				slot = atoi(string + 7);

				if (pdis->itemState & ODS_SELECTED) {
					back_colour = RGB(0, 0, 255);
					back_brush = CreateSolidBrush(back_colour);
					FillRect(pdis->hDC, &pdis->rcItem, back_brush);
					DeleteObject(back_brush);
					SetBkColor(pdis->hDC, back_colour);
				} else {
					back_colour = RGB(255, 255, 255);
					back_brush = CreateSolidBrush(back_colour);
					FillRect(pdis->hDC, &pdis->rcItem, back_brush);
					DeleteObject(back_brush);
					SetBkColor(pdis->hDC, back_colour);
				}

				joyInfo.dwFlags = JOY_RETURNBUTTONS;
				joyInfo.dwSize = sizeof(joyInfo);

				if (cfg_std_ctrl.cfg.id == 1) {
					if (slot == cfg_port2.port.joyID) {
						same_port = TRUE;
					}
				} else if (cfg_std_ctrl.cfg.id == 2) {
					if (slot == cfg_port1.port.joyID) {
						same_port = TRUE;
					}
				}

				if ((joyGetPosEx(slot, &joyInfo) == JOYERR_NOERROR) && !same_port) {
					SetTextColor(pdis->hDC, RGB(0, 0, 0));
					cfg_std_ctrl.joy_connected[slot] = TRUE;
				} else {
					SetTextColor(pdis->hDC, RGB(200, 200, 200));
					cfg_std_ctrl.joy_connected[slot] = FALSE;
				}

				DrawText(pdis->hDC, string, strlen(string), &pdis->rcItem, DT_LEFT | DT_SINGLELINE);
			}
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);
					memcpy(cfg_std_ctrl.cfgport, &cfg_std_ctrl.cfg,
							sizeof(cfg_std_ctrl.cfg));
					return TRUE;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				case IDC_STD_CTRL_JOY_ID:
					if (HIWORD(wParam) == CBN_SELCHANGE) {

						cfg_std_ctrl.cfg.port.joyID = SendDlgItemMessage(hwnd,
								IDC_STD_CTRL_JOY_ID, CB_GETCURSEL, 0, 0);
					}
					return TRUE;
				case IDC_STD_CTRL_KEY_A:
				case IDC_STD_CTRL_KEY_B:
				case IDC_STD_CTRL_KEY_SELECT:
				case IDC_STD_CTRL_KEY_START:
				case IDC_STD_CTRL_KEY_UP:
				case IDC_STD_CTRL_KEY_DOWN:
				case IDC_STD_CTRL_KEY_LEFT:
				case IDC_STD_CTRL_KEY_RIGHT:
				case IDC_STD_CTRL_KEY_TURBOA:
				case IDC_STD_CTRL_KEY_TURBOB: {
					int bt = LOWORD(wParam);

					if (cfg_std_ctrl.no_other_buttons) {
						SetFocus(cfg_std_ctrl.button_pressed);
						return TRUE;
					}

					cfg_std_ctrl.no_other_buttons = TRUE;

					SetDlgItemText(hwnd, bt, "press a key");

					cfg_std_ctrl.button_pressed = GetDlgItem(hwnd, bt);
					cfg_std_ctrl.controller_input = bt - IDC_STD_CTRL_KEY_A;

					cfg_std_ctrl.old_wnd_proc = (WNDPROC) SetWindowLongPtr(
							cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
							(LONG_PTR) cfg_standard_controller_read_kbd);
					return TRUE;
				}
				case IDC_STD_CTRL_JOY_A:
				case IDC_STD_CTRL_JOY_B:
				case IDC_STD_CTRL_JOY_SELECT:
				case IDC_STD_CTRL_JOY_START:
				case IDC_STD_CTRL_JOY_UP:
				case IDC_STD_CTRL_JOY_DOWN:
				case IDC_STD_CTRL_JOY_LEFT:
				case IDC_STD_CTRL_JOY_RIGHT:
				case IDC_STD_CTRL_JOY_TURBOA:
				case IDC_STD_CTRL_JOY_TURBOB: {
					int bt = LOWORD(wParam);

					if (cfg_std_ctrl.no_other_buttons) {
						SetFocus(cfg_std_ctrl.button_pressed);
						return TRUE;
					}

					/* se il joystick non e' collegato non faccio niente */
					if (cfg_std_ctrl.joy_connected[cfg_std_ctrl.cfg.port.joyID]
					                               == FALSE) {
						return TRUE;
					}
					/*
					 * i due controller non possono
					 * utilizzare lo stesso device ed
					 * il primo ha priorita' sul secondo.
					 */
					if (cfg_std_ctrl.cfg.id == 2) {
						if (cfg_std_ctrl.cfg.port.joyID == cfg_port1.port.joyID) {
							return TRUE;
						}
					}

					cfg_std_ctrl.no_other_buttons = TRUE;

					SetDlgItemText(hwnd, bt, "press a key");

					cfg_std_ctrl.button_pressed = GetDlgItem(hwnd, bt);
					cfg_std_ctrl.controller_input = bt - IDC_STD_CTRL_JOY_A;

					cfg_std_ctrl.old_wnd_proc = (WNDPROC) SetWindowLongPtr(
							cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
							(LONG_PTR) cfg_standard_controller_joy_esc);

					SetTimer(hwnd, IDT_TIMER2, 150,
							(TIMERPROC) cfg_standard_controller_read_joy);
					return TRUE;
				}
			}
			return FALSE;
	}
	return FALSE;
}
long __stdcall cfg_standard_controller_read_kbd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN: {
			int keyval = LOWORD(wParam);

			if (keyval == VK_ESCAPE) {
				cfgportinput(KEYBOARD) = 0;
			} else {
				if (cfg_standard_controller_input_is_not_ok(keyval, KEYBOARD)) {
					SetWindowText(hwnd, cazzata[(WORD) (rand() % 8) & 0x07]);
					break;
				} else {
					cfgportinput(KEYBOARD) = LOWORD(wParam);
				}
			}

			SetWindowText(hwnd, keyvalToName(cfgportinput(KEYBOARD)));

			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) cfg_std_ctrl.old_wnd_proc);

			cfg_std_ctrl.old_wnd_proc = NULL;
			cfg_std_ctrl.no_other_buttons = FALSE;
			cfg_std_ctrl.controller_input = 0;
		}
		break;
	}
	return CallWindowProc(cfg_std_ctrl.old_wnd_proc, hwnd, msg, wParam, lParam);
}
void __stdcall cfg_standard_controller_read_joy(void) {
	JOYINFOEX joyInfo;
	JOYCAPS joyCaps;
	WORD value = 0;

	/*
	 * i due controller non possono
	 * utilizzare lo stesso device. ed
	 */
	if (cfg_std_ctrl.cfg.id == 1) {
		if (cfg_std_ctrl.cfg.port.joyID == cfg_port2.port.joyID) {
			return;
		}
	} else if (cfg_std_ctrl.cfg.id == 2) {
		if (cfg_std_ctrl.cfg.port.joyID == cfg_port1.port.joyID) {
			return;
		}
	}

	joyGetDevCaps(cfg_std_ctrl.cfg.port.joyID, &joyCaps, sizeof(joyCaps));

	joyInfo.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_USEDEADZONE;
	joyInfo.dwSize = sizeof(joyInfo);

	if (joyGetPosEx(cfg_std_ctrl.cfg.port.joyID, &joyInfo) != JOYERR_NOERROR) {
		return;
	}

	/* esamino i pulsanti */
	if (joyInfo.dwButtons) {
		BYTE i;

		for (i = 0; i < MAXBUTTONS; i++) {
			BYTE button = joyInfo.dwButtons & 0x1;

			if (button) {
				value = i | 0x400;
				goto elaborateValue;
			}
			joyInfo.dwButtons >>= 1;
		}
	}

	/* esamino gli assi */
	if (joyInfo.dwXpos != CENTER) {
		read_axis_joy(X, dwXpos);
	} else if (joyInfo.dwYpos != CENTER) {
		read_axis_joy(Y, dwYpos);
	} else if ((joyCaps.wCaps & JOYCAPS_HASZ) && (joyInfo.dwZpos != CENTER)) {
		read_axis_joy(Z, dwZpos);
	} else if ((joyCaps.wCaps & JOYCAPS_HASR) && (joyInfo.dwRpos != CENTER)) {
		read_axis_joy(R, dwRpos);
	} else if ((joyCaps.wCaps & JOYCAPS_HASU) && (joyInfo.dwUpos)) {
		/* FIXME: non so bene come funzionano gli assi U e V */
		//read_axis_joy(U, dwUpos);
	} else if ((joyCaps.wCaps & JOYCAPS_HASV) && (joyInfo.dwVpos)) {
		//read_axis_joy(V, dwVpos);
	}

	elaborateValue: if (value) {
		if (cfg_standard_controller_input_is_not_ok(value, JOYSTICK)) {
			SetWindowText(cfg_std_ctrl.button_pressed, cazzata[(WORD) (rand() % 8) & 0x07]);
			return;
		}

		KillTimer(cfg_std_ctrl.toplevel, IDT_TIMER2);

		cfgportinput(JOYSTICK) = value;

		SetWindowText(cfg_std_ctrl.button_pressed, jsvToName(cfgportinput(JOYSTICK)));

		SetWindowLongPtr(cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
				(LONG_PTR) cfg_std_ctrl.old_wnd_proc);

		cfg_std_ctrl.old_wnd_proc = NULL;
		cfg_std_ctrl.no_other_buttons = FALSE;
		cfg_std_ctrl.controller_input = 0;
	}
}
long __stdcall cfg_standard_controller_joy_esc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN:
			if (LOWORD(wParam) == VK_ESCAPE) {
				KillTimer(cfg_std_ctrl.toplevel, IDT_TIMER2);

				cfgportinput(JOYSTICK) = 0;

				SetWindowText(hwnd, keyvalToName(cfgportinput(JOYSTICK)));

				SetWindowLongPtr(cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
						(LONG_PTR) cfg_std_ctrl.old_wnd_proc);

				cfg_std_ctrl.old_wnd_proc = NULL;
				cfg_std_ctrl.no_other_buttons = FALSE;
				cfg_std_ctrl.controller_input = 0;
			}
			break;
	}
	return CallWindowProc(cfg_std_ctrl.old_wnd_proc, hwnd, msg, wParam, lParam);
}
BOOL cfg_standard_controller_input_is_not_ok(DBWORD input, BYTE type) {
	_cfg_port *p1, *p2;
	BYTE i;

	p1 = &cfg_std_ctrl.cfg;

	if (cfg_std_ctrl.cfg.id == 1) {
		p2 = &cfg_port2;
	} else {
		p2 = &cfg_port1;
	}

	if (type == KEYBOARD) {
		/* keyboard */
		for (i = 0; i < maxButtons; i++) {
			if ((p1->port.input[KEYBOARD][i] == input) || (p2->port.input[KEYBOARD][i] == input)) {
				if (cfg_std_ctrl.controller_input == i) {
					return (EXIT_OK);
				} else {
					return (EXIT_ERROR);
				}
			}
		}
	} else {
		/* joystick */
		for (i = 0; i < maxButtons; i++) {
			if (p2->port.input[JOYSTICK][i] == input) {
				if (p1->port.joyID == p2->port.joyID) {
					return (EXIT_ERROR);
				}
			}
			if (p1->port.input[JOYSTICK][i] == input) {
				if (cfg_std_ctrl.controller_input == i) {
					return (EXIT_OK);
				} else {
					return (EXIT_ERROR);
				}
			}
		}
	}
	return (EXIT_OK);
}
