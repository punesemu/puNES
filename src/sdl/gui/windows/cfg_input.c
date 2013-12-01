/*
 * cfg_input.c
 *
 *  Created on: 27/nov/2013
 *      Author: fhorse
 */

#include "cfg_input.h"
#include "cfg_std_pad.h"
#include "opengl.h"
#include "cfg_file.h"

long __stdcall cfg_input_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void cfg_input_destory(HWND hwnd, INT_PTR result);
void cfg_input_update_dialog(HWND hwnd);
void cfg_input_setup_button_enable(HWND hwnd, _port *port, int button);

void cfg_input_dialog(HWND hwnd) {
	memset(&cfg_input, 0x00, sizeof(cfg_input));

	memcpy(&cfg_input.settings, &cfg->input, sizeof(_config_input));

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			cfg_input.port[i].id = i + 1;
			memcpy(&cfg_input.port[i].port, &port[i], sizeof(_port));
		}
	}

	/* Faccio l'update del menu per i casi dello zapper e degli effetti */
	gui_update();

	emu_pause(TRUE);

	cfg_input.father = CreateDialog(GetModuleHandle(NULL ),
			MAKEINTRESOURCE(IDD_INPUT_DIALOG), hwnd, (DLGPROC) cfg_input_messages);
}

long __stdcall cfg_input_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			cfg_input_destory(hwnd, 0);
			return (TRUE);
		case WM_INITDIALOG: {
			static struct _ctrl_list_element {
				int type;
				char name[20];
			} ctrl_list[] = {
				{ CTRL_DISABLED, "Disabled"     },
				{ CTRL_STANDARD, "Standard Pad" },
				{ CTRL_ZAPPER,   "Zapper"       }
			};
			BYTE a;

			for (a = PORT1; a < PORT_MAX; a++) {
				BYTE b;

				SendDlgItemMessage(hwnd, IDC_INPUT_CONTROLLER1_COMBO + a, CB_RESETCONTENT, 0, 0);

				for (b = 0; b < LENGTH(ctrl_list); b++) {
					SendDlgItemMessage(hwnd, IDC_INPUT_CONTROLLER1_COMBO + a, CB_ADDSTRING, 0,
					        (LPARAM) ctrl_list[b].name);
				}
			}

			cfg_input_update_dialog(hwnd);

			return (TRUE);
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case ID_INPUT_DEFAULT: {
					BYTE i;
					_array_pointers_port array;

					for (i = PORT1; i < PORT_MAX; i++) {
						array.port[i] = &cfg_input.port[i].port;
					}

					cfg_file_set_all_input_default(&cfg_input.settings, &array);

					cfg_input_update_dialog(hwnd);
					return (TRUE);
				}
				case IDOK:
					cfg_input_destory(hwnd, IDOK);

					if (opengl.rotation
					        && (input_zapper_is_connected((_port *) &cfg_input.port) == TRUE)) {
						set_effect();
					}

					memcpy(&cfg->input, &cfg_input.settings, sizeof(_config_input));

					{
						BYTE i;

						for (i = PORT1; i < PORT_MAX; i++) {
							if (cfg_input.port[i].port.type != port[i].type) {
								BYTE a;

								for (a = TRB_A; a <= TRB_B; a++) {
									int type = a - TRB_A;

									cfg_input.port[i].port.turbo[type].active = 0;
									cfg_input.port[i].port.turbo[type].counter = 0;
								}
							}
							memcpy(&port[i], &cfg_input.port[i].port, sizeof(_port));
						}
					}

					/* Faccio l'update del menu per i casi dello zapper e degli effetti */
					gui_update();

					cfg_file_input_save();

					input_init();

					js_quit();
					js_init();
					return (TRUE);
				case IDCANCEL:
					cfg_input_destory(hwnd, IDCANCEL);
					return (TRUE);
				case IDC_INPUT_CONTROLLER1_COMBO:
				case IDC_INPUT_CONTROLLER2_COMBO:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						BYTE index = LOWORD(wParam) - IDC_INPUT_CONTROLLER1_COMBO;

						cfg_input.port[index].port.type = SendDlgItemMessage(hwnd,
								IDC_INPUT_CONTROLLER1_COMBO + index, CB_GETCURSEL, 0, 0);
						cfg_input_setup_button_enable(hwnd, &cfg_input.port[index].port,
								IDC_INPUT_CONTROLLER1_SETUP_BUTTON + index);
					}
					return (TRUE);
				case IDC_INPUT_CONTROLLER1_SETUP_BUTTON:
				case IDC_INPUT_CONTROLLER2_SETUP_BUTTON: {
					BYTE index = LOWORD(wParam) - IDC_INPUT_CONTROLLER1_SETUP_BUTTON;

					cfg_std_pad_dialog(hwnd, &cfg_input.port[index]);
					return (TRUE);
				}
				case IDC_PERMIT_UPDOWN_LEFTRIGHT_CHECKBOX:
					cfg_input.settings.permit_updown_leftright =
							!cfg_input.settings.permit_updown_leftright;
					return (TRUE);
				case IDC_CHECK_CONFLICTS_CHECKBOX:
					cfg_input.settings.check_input_conflicts =
							!cfg_input.settings.check_input_conflicts;

					/* faccio il check dell'input */
					if (cfg_input.settings.check_input_conflicts == TRUE) {
						BYTE i;
						_array_pointers_port array;

						for (i = PORT1; i < PORT_MAX; i++) {
							array.port[i] = &cfg_input.port[i].port;
						}
						input_check_conflicts(&cfg_input.settings, &array);
					}
					return (TRUE);
			}
			break;
		}
	}

	return (FALSE);
}
void cfg_input_destory(HWND hwnd, INT_PTR result) {
	EndDialog(hwnd, result);
	emu_pause(FALSE);
}
void cfg_input_update_dialog(HWND hwnd) {
	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			SendDlgItemMessage(hwnd, IDC_INPUT_CONTROLLER1_COMBO + i, CB_SETCURSEL,
					cfg_input.port[i].port.type, 0);

			cfg_input_setup_button_enable(hwnd, &cfg_input.port[i].port,
					IDC_INPUT_CONTROLLER1_SETUP_BUTTON + i);
		}
	}

	{
		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_CONTROLLER4_LABEL_STATIC), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_CONTROLLER4_COMBO), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_CONTROLLER4_SETUP_BUTTON), FALSE);

		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_CONTROLLER3_LABEL_STATIC), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_CONTROLLER3_COMBO), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_CONTROLLER3_SETUP_BUTTON), FALSE);

		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_EXPANSION_LABEL_STATIC), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_INPUT_EXPANSION_COMBO), FALSE);
	}

	if (cfg_input.settings.permit_updown_leftright == TRUE) {
		SendDlgItemMessage(hwnd, IDC_PERMIT_UPDOWN_LEFTRIGHT_CHECKBOX, BM_SETCHECK,
		        (WPARAM) BST_CHECKED, 0);
	} else {
		SendDlgItemMessage(hwnd, IDC_PERMIT_UPDOWN_LEFTRIGHT_CHECKBOX, BM_SETCHECK,
		        (WPARAM) BST_UNCHECKED, 0);
	}


	if (cfg_input.settings.check_input_conflicts == TRUE) {
		SendDlgItemMessage(hwnd, IDC_CHECK_CONFLICTS_CHECKBOX, BM_SETCHECK,
		        (WPARAM) BST_UNCHECKED, 0);
	} else {
		SendDlgItemMessage(hwnd, IDC_CHECK_CONFLICTS_CHECKBOX, BM_SETCHECK,
		        (WPARAM) BST_CHECKED, 0);
	}
}
void cfg_input_setup_button_enable(HWND hwnd, _port *port, int button) {
	switch (port->type) {
		case CTRL_DISABLED:
		case CTRL_ZAPPER:
			EnableWindow(GetDlgItem(hwnd, button), FALSE);
			break;
		case CTRL_STANDARD:
			EnableWindow(GetDlgItem(hwnd, button), TRUE);
			break;
	}
}
