/*
 * cfg_std_ctrl.c
 *
 *  Created on: 04/nov/2011
 *      Author: fhorse
 */

#include "win.h"
#include "cfg_std_ctrl.h"
#include "commctrl.h"
#include "param.h"

long __stdcall cfg_standard_controller_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_standard_controller_read_kbd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
long __stdcall cfg_standard_controller_joy_esc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __stdcall cfg_standard_controller_read_joy(void);
BOOL cfg_standard_controller_input_is_not_ok(DBWORD input, BYTE type);
void cfg_standard_controller_joy_timer_kill(void);

#define cfg_port_input(type)\
	cfg_std_ctrl.cfg.port.input[type][cfg_std_ctrl.controller_input]
#define read_axis_joy(axis, info)\
	value = (axis << 1) + 1;\
	if (joy_info.info > CENTER) {\
		value++;\
	}

typedef struct {
	HWND toplevel;
	HWND button_pressed;
	WNDPROC old_wnd_proc;
	BYTE no_other_buttons;
	BYTE controller_input;
	BYTE joy_connected[JOYSTICKID15];
	BYTE joy_timer_run;
	_cfg_port cfg;
	_cfg_port *cfg_port;
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

BYTE max_buttons = LENGTH(param_input_p1k);

void cfg_standard_controller(HWND hwnd, _cfg_port *cfg_port) {
	char title[30];

	memset(&cfg_std_ctrl, 0, sizeof(cfg_std_ctrl));
	memcpy(&cfg_std_ctrl.cfg, cfg_port, sizeof(cfg_std_ctrl.cfg));

	cfg_std_ctrl.cfg_port = cfg_port;

	cfg_std_ctrl.toplevel = CreateDialog(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_STD_CTRL), hwnd,
			(DLGPROC) cfg_standard_controller_wnd_proc);

	sprintf(title, "Controller %d setup", cfg_std_ctrl.cfg.id);

	SetWindowText(cfg_std_ctrl.toplevel, title);
}
long __stdcall cfg_standard_controller_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DRAWITEMSTRUCT *pdis;

	switch (msg) {
		case WM_NOTIFY: {
			switch (((LPNMHDR) lParam)->code) {
				case TTN_NEEDTEXT: {
					LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT) lParam;
					int ctrlID = GetDlgCtrlID((HWND) wParam);
					int value = SendDlgItemMessage(hwnd, ctrlID, TBM_GETPOS, 0, 0);
					static char szBuf[80] = "";

					sprintf(szBuf, "%d", value);
					lpToolTipText->lpszText = szBuf;
					break;
				}
			}
			break;
		}
		case WM_INITDIALOG: {
			int i;

			for (i = 0; i < max_buttons; i++) {
				SetDlgItemText(hwnd, IDC_STD_CTRL_KEY_A + i,
				        keyval_to_name(cfg_std_ctrl.cfg.port.input[KEYBOARD][i]));
				SetDlgItemText(hwnd, IDC_STD_CTRL_JOY_A + i,
				        jsv_to_name(cfg_std_ctrl.cfg.port.input[JOYSTICK][i]));
			}

			SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_RESETCONTENT, 0, 0);

			for (i = 0; i <= 4; i++) {
				char label[30];

				if (i < 4) {
					sprintf(label, "Device %d", i);
				} else {
					sprintf(label, "Disabled");
				}

				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_ADDSTRING, i, (LPARAM) label);
			}

			if (cfg_std_ctrl.cfg.port.joy_id == name_to_jsn("NULL")) {
				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_SETCURSEL, 4, 0);
			} else {
				SendDlgItemMessage(hwnd, IDC_STD_CTRL_JOY_ID, CB_SETCURSEL,
						cfg_std_ctrl.cfg.port.joy_id, 0);
			}

			for (i = IDC_TURBOA_DELAY_SLIDER; i <= IDC_TURBOB_DELAY_SLIDER; i++) {
				int pos = cfg_std_ctrl.cfg.port.turbo[i - IDC_TURBOA_DELAY_SLIDER].frequency;

				SendDlgItemMessage(hwnd, i, TBM_SETRANGE, (WPARAM) TRUE,
				        (LPARAM) MAKELONG(1, TURBO_BUTTON_DELAY_MAX));
				SendDlgItemMessage(hwnd, i, TBM_SETTIC, 0, (LPARAM) TURBO_BUTTON_DELAY_DEFAULT);
				SendDlgItemMessage(hwnd, i, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) pos);
			}

			return (TRUE);
		}
		case WM_DRAWITEM:
			pdis = (DRAWITEMSTRUCT *) lParam;

			if (pdis->CtlID == IDC_STD_CTRL_JOY_ID) {
				JOYINFOEX joy_info;
				HBRUSH back_brush;
				COLORREF back_colour;
				char string[30];
				int slot = 0;
				BYTE same_port = FALSE;

				if (pdis->itemID == -1) {
					return (TRUE);
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

				if (strcmp(string, "Disabled") == 0) {
					SetTextColor(pdis->hDC, RGB(0, 0, 0));
					DrawText(pdis->hDC, string, strlen(string), &pdis->rcItem,
					        DT_LEFT | DT_SINGLELINE);
					return (FALSE);
				}

				joy_info.dwFlags = JOY_RETURNBUTTONS;
				joy_info.dwSize = sizeof(joy_info);

				if ((cfg_port1.port.type == cfg_port2.port.type)
				        && ((cfg_port1.port.type != CTRL_STANDARD)
				                && (cfg_port2.port.type != CTRL_STANDARD))) {
					if (cfg_std_ctrl.cfg.id == 1) {
						if (slot == cfg_port2.port.joy_id) {
							same_port = TRUE;
						}
					} else if (cfg_std_ctrl.cfg.id == 2) {
						if (slot == cfg_port1.port.joy_id) {
							same_port = TRUE;
						}
					}
				}

				if ((joyGetPosEx(slot, &joy_info) == JOYERR_NOERROR) && !same_port) {
					SetTextColor(pdis->hDC, RGB(0, 0, 0));
					cfg_std_ctrl.joy_connected[slot] = TRUE;
				} else {
					SetTextColor(pdis->hDC, RGB(200, 200, 200));
					cfg_std_ctrl.joy_connected[slot] = FALSE;
				}

				DrawText(pdis->hDC, string, strlen(string), &pdis->rcItem, DT_LEFT | DT_SINGLELINE);
			}
			return (FALSE);
		case WM_HSCROLL: {
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			int type = ctrlID - IDC_TURBOA_DELAY_SLIDER;
			int value = SendDlgItemMessage(hwnd, ctrlID, TBM_GETPOS, 0, 0);

			cfg_std_ctrl.cfg.port.turbo[type].frequency = value;
			cfg_std_ctrl.cfg.port.turbo[type].counter = 0;

			return (TRUE);
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					if (cfg_std_ctrl.joy_timer_run == TRUE) {
						cfg_standard_controller_joy_timer_kill();
					}
					EndDialog(hwnd, IDOK);
					memcpy(cfg_std_ctrl.cfg_port, &cfg_std_ctrl.cfg,
							sizeof(cfg_std_ctrl.cfg));
					return (TRUE);
				case IDCANCEL:
					if (cfg_std_ctrl.joy_timer_run == TRUE) {
						cfg_standard_controller_joy_timer_kill();
					}
					EndDialog(hwnd, IDCANCEL);
					return (TRUE);
				case IDC_STD_CTRL_JOY_ID:
					if (cfg_std_ctrl.no_other_buttons) {
						SetFocus(cfg_std_ctrl.button_pressed);
						return (TRUE);
					}
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						cfg_std_ctrl.cfg.port.joy_id = SendDlgItemMessage(hwnd,
								IDC_STD_CTRL_JOY_ID, CB_GETCURSEL, 0, 0);
						if (cfg_std_ctrl.cfg.port.joy_id == 4) {
							cfg_std_ctrl.cfg.port.joy_id = name_to_jsn("NULL");
						}
					}
					return (TRUE);
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
						return (TRUE);
					}

					cfg_std_ctrl.no_other_buttons = TRUE;

					SetDlgItemText(hwnd, bt, "press a key");

					cfg_std_ctrl.button_pressed = GetDlgItem(hwnd, bt);
					cfg_std_ctrl.controller_input = bt - IDC_STD_CTRL_KEY_A;

					cfg_std_ctrl.old_wnd_proc = (WNDPROC) SetWindowLongPtr(
							cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
							(LONG_PTR) cfg_standard_controller_read_kbd);
					return (TRUE);
				}
				case IDC_STD_CTRL_KEY_ERASE: {
					int i;

					if (cfg_std_ctrl.no_other_buttons) {
						SetFocus(cfg_std_ctrl.button_pressed);
						return (TRUE);
					}
					for (i = IDC_STD_CTRL_KEY_A; i <= IDC_STD_CTRL_KEY_TURBOB; i++) {
						cfg_std_ctrl.cfg.port.input[KEYBOARD][i - IDC_STD_CTRL_KEY_A] = 0;
						SetDlgItemText(hwnd, i, "NULL");
					}
					return (TRUE);
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
						return (TRUE);
					}
					/* se il joystick e' disabilitato non faccio niente */
					if (cfg_std_ctrl.cfg.port.joy_id == name_to_jsn("NULL")) {
						return (TRUE);
					}
					/* se il joystick non e' collegato non faccio niente */
					if (cfg_std_ctrl.joy_connected[cfg_std_ctrl.cfg.port.joy_id] == FALSE) {
						return (TRUE);
					}
					/*
					 * i due controller non possono
					 * utilizzare lo stesso device ed
					 * il primo ha priorita' sul secondo.
					 */
					if (cfg_std_ctrl.cfg.id == 2) {
						if (cfg_std_ctrl.cfg.port.joy_id == cfg_port1.port.joy_id) {
							return (TRUE);
						}
					}

					cfg_std_ctrl.no_other_buttons = TRUE;

					SetDlgItemText(hwnd, bt, "press a key");

					cfg_std_ctrl.button_pressed = GetDlgItem(hwnd, bt);
					cfg_std_ctrl.controller_input = bt - IDC_STD_CTRL_JOY_A;

					cfg_std_ctrl.old_wnd_proc = (WNDPROC) SetWindowLongPtr(
							cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
							(LONG_PTR) cfg_standard_controller_joy_esc);

					cfg_std_ctrl.joy_timer_run = TRUE;

					SetTimer(hwnd, IDT_TIMER2, 150,
							(TIMERPROC) cfg_standard_controller_read_joy);
					return (TRUE);
				}
				case IDC_STD_CTRL_JOY_ERASE: {
					int i;

					if (cfg_std_ctrl.no_other_buttons) {
						SetFocus(cfg_std_ctrl.button_pressed);
						return (TRUE);
					}
					for (i = IDC_STD_CTRL_JOY_A; i <= IDC_STD_CTRL_JOY_TURBOB; i++) {
						cfg_std_ctrl.cfg.port.input[JOYSTICK][i - IDC_STD_CTRL_JOY_A] = 0;
						SetDlgItemText(hwnd, i, "NULL");
					}
					return (TRUE);
				}
			}
			return (FALSE);
	}
	return (FALSE);
}
long __stdcall cfg_standard_controller_read_kbd(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN: {
			int keyval = LOWORD(wParam);

			if (keyval == VK_ESCAPE) {
				cfg_port_input(KEYBOARD) = 0;
			} else {
				if (cfg_standard_controller_input_is_not_ok(keyval, KEYBOARD)) {
					SetWindowText(hwnd, cazzata[(WORD) (rand() % 8) & 0x07]);
					break;
				} else {
					cfg_port_input(KEYBOARD) = LOWORD(wParam);
				}
			}

			SetWindowText(hwnd, keyval_to_name(cfg_port_input(KEYBOARD)));

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
	JOYINFOEX joy_info;
	JOYCAPS joy_caps;
	WORD value = 0;

	/*
	 * i due controller non possono
	 * utilizzare lo stesso device. ed
	 */
	if ((cfg_port1.port.type == cfg_port2.port.type)
	        && ((cfg_port1.port.type != CTRL_STANDARD) && (cfg_port2.port.type != CTRL_STANDARD))) {
		if (cfg_std_ctrl.cfg.id == 1) {
			if (cfg_std_ctrl.cfg.port.joy_id == cfg_port2.port.joy_id) {
				return;
			}
		} else if (cfg_std_ctrl.cfg.id == 2) {
			if (cfg_std_ctrl.cfg.port.joy_id == cfg_port1.port.joy_id) {
				return;
			}
		}
	}

	joyGetDevCaps(cfg_std_ctrl.cfg.port.joy_id, &joy_caps, sizeof(joy_caps));

	joy_info.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_RETURNPOV | JOY_USEDEADZONE;
	joy_info.dwSize = sizeof(joy_info);

	if (joyGetPosEx(cfg_std_ctrl.cfg.port.joy_id, &joy_info) != JOYERR_NOERROR) {
		return;
	}

	/* esamino i pulsanti */
	if (joy_info.dwButtons) {
		BYTE i;

		for (i = 0; i < MAX_BUTTONS; i++) {
			BYTE button = joy_info.dwButtons & 0x1;

			if (button) {
				value = i | 0x400;
				goto elaborateValue;
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

	elaborateValue:
	if (value) {
		if (cfg_standard_controller_input_is_not_ok(value, JOYSTICK)) {
			SetWindowText(cfg_std_ctrl.button_pressed, cazzata[(WORD) (rand() % 8) & 0x07]);
			return;
		}
		cfg_port_input(JOYSTICK) = value;
		SetWindowText(cfg_std_ctrl.button_pressed, jsv_to_name(cfg_port_input(JOYSTICK)));
		cfg_standard_controller_joy_timer_kill();
	}
}
long __stdcall cfg_standard_controller_joy_esc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KEYDOWN:
			if (LOWORD(wParam) == VK_ESCAPE) {
				cfg_port_input(JOYSTICK) = 0;
				SetWindowText(hwnd, keyval_to_name(cfg_port_input(JOYSTICK)));
				cfg_standard_controller_joy_timer_kill();
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
		for (i = 0; i < max_buttons; i++) {
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
		for (i = 0; i < max_buttons; i++) {
			if (p2->port.input[JOYSTICK][i] == input) {
				if (p1->port.joy_id == p2->port.joy_id) {
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
void cfg_standard_controller_joy_timer_kill(void) {
	KillTimer(cfg_std_ctrl.toplevel, IDT_TIMER2);

	SetWindowLongPtr(cfg_std_ctrl.button_pressed, GWLP_WNDPROC,
	        (LONG_PTR) cfg_std_ctrl.old_wnd_proc);

	cfg_std_ctrl.old_wnd_proc = NULL;
	cfg_std_ctrl.no_other_buttons = FALSE;
	cfg_std_ctrl.controller_input = 0;

	cfg_std_ctrl.joy_timer_run = FALSE;
}
