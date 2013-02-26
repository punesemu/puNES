/*
 * cfginput.c
 *
 *  Created on: 04/nov/2011
 *      Author: fhorse
 */

#include "win.h"
#include "cfgstdctrl.h"
#include "sdlsnd.h"
#include "cfg_file.h"
#include "sdl_gfx.h"
#include "opengl.h"

#define cfginput_enable_config(prt, idc)\
	switch (prt.port.type) {\
		case CTRLDISABLED:\
		case ZAPPER:\
			EnableWindow(GetDlgItem(hwnd, idc), FALSE);\
			break;\
		case STDCTRL:\
			EnableWindow(GetDlgItem(hwnd, idc), TRUE);\
			break;\
	}

typedef struct {
	DBWORD controller;
	DBWORD value;
	char name[20];
} _typeselement;

static const _typeselement ctrlList[] = {
	{ 0, CTRLDISABLED, "Disabled"     },
	{ 0, STDCTRL,      "Standard Pad" },
	{ 0, ZAPPER,       "Zapper"       }
};

long __stdcall cfg_input_controllers(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void cfgInput(HWND hwnd) {
	cfg_port1.id = 1;
	memcpy(&cfg_port1.port, &port1, sizeof(port1));
	cfg_port2.id = 2;
	memcpy(&cfg_port2.port, &port2, sizeof(port2));

	/* Faccio l'update del menu per i casi dello zapper e degli effetti */
	guiUpdate();

	emu_pause(TRUE);

	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INPUT_CONTROLLERS),
			hwnd, (DLGPROC) cfg_input_controllers);
}
long __stdcall cfg_input_controllers(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY:
			emu_pause(FALSE);
			PostQuitMessage(0);
			return TRUE;
		case WM_INITDIALOG: {
			BYTE i;

			SendDlgItemMessage(hwnd, IDC_INPUT_CTRL_PORT1_C, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hwnd, IDC_INPUT_CTRL_PORT2_C, CB_RESETCONTENT, 0, 0);

			for (i = 0; i < LENGTH(ctrlList); i++) {
				SendDlgItemMessage(hwnd, IDC_INPUT_CTRL_PORT1_C, CB_ADDSTRING, 0,
						(LPARAM) ctrlList[i].name);
				SendDlgItemMessage(hwnd, IDC_INPUT_CTRL_PORT2_C, CB_ADDSTRING, 0,
						(LPARAM) ctrlList[i].name);
			}
			SendDlgItemMessage(hwnd, IDC_INPUT_CTRL_PORT1_C, CB_SETCURSEL, cfg_port1.port.type, 0);
			SendDlgItemMessage(hwnd, IDC_INPUT_CTRL_PORT2_C, CB_SETCURSEL, cfg_port2.port.type, 0);

			cfginput_enable_config(cfg_port1, IDC_INPUT_CTRL_PORT1_B);
			cfginput_enable_config(cfg_port2, IDC_INPUT_CTRL_PORT2_B);

			return TRUE;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);

					if (((cfg_port1.port.type == ZAPPER) || (cfg_port2.port.type == ZAPPER))
							&& opengl.rotation) {
						set_effect();
					}

					if (cfg_port1.port.type != port1.type) {
						cfg_port1.port.changed = TRUE;
					} else {
						cfg_port1.port.changed = FALSE;
					}

					if (cfg_port2.port.type != port2.type) {
						cfg_port2.port.changed = TRUE;
					} else {
						cfg_port2.port.changed = FALSE;
					}

					memcpy(&port1, &cfg_port1.port, sizeof(port1));
					memcpy(&port2, &cfg_port2.port, sizeof(port2));

					/* Faccio l'update del menu per i casi dello zapper e degli effetti */
					guiUpdate();

					cfg_file_input_save();

					input_init();

					jsQuit();
					jsInit();
					return TRUE;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				case IDC_INPUT_CTRL_PORT1_C:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						cfg_port1.port.type = SendDlgItemMessage(hwnd,
								IDC_INPUT_CTRL_PORT1_C, CB_GETCURSEL, 0, 0);

						cfginput_enable_config(cfg_port1, IDC_INPUT_CTRL_PORT1_B);
					}
					return TRUE;
				case IDC_INPUT_CTRL_PORT2_C:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						cfg_port2.port.type = SendDlgItemMessage(hwnd,
								IDC_INPUT_CTRL_PORT2_C, CB_GETCURSEL, 0, 0);

						cfginput_enable_config(cfg_port2, IDC_INPUT_CTRL_PORT2_B);
					}
					return TRUE;
				case IDC_INPUT_CTRL_PORT1_B:
					cfg_standard_controller(hwnd, &cfg_port1);
					return TRUE;
				case IDC_INPUT_CTRL_PORT2_B:
					cfg_standard_controller(hwnd, &cfg_port2);
					return TRUE;
			}
			break;
		}
	}
	return FALSE;
}
