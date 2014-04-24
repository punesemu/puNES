/*
 * cfg_overscan_borders.c
 *
 *  Created on: 24/mar/2014
 *      Author: fhorse
 */

#include "cfg_overscan_borders.h"
#include <commctrl.h>
#include "overscan.h"
#include "cfg_file.h"
#include "param.h"
#include "clock.h"
#include "gfx.h"

long __stdcall cfg_overscan_borders_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void cfg_overscan_borders_update_dialog(HWND hwnd);
void cfg_overscan_borders_destory(HWND hwnd, INT_PTR result);

struct _cfg_overscan_borders {
	HWND father;

	BYTE save_overscan;
	BYTE force_set_mode;

	int mode;
	int initdialog;

	_overscan_borders save_borders;
	_overscan_borders preview;
	_overscan_borders overscan_borders[2];
	_overscan_borders *borders;

} cfg_oscan;

void cfg_overscan_borders_dialog(HWND hwnd) {
	memset(&cfg_oscan, 0x00, sizeof(cfg_oscan));

	memcpy(&cfg_oscan.overscan_borders, &overscan_borders, sizeof(overscan_borders));

	/*
	 * salvo sia il parametro dell'overscan
	 * che il settaggio attuale dei bordi.
	 */
	cfg_oscan.save_overscan = cfg->oscan;
	cfg_oscan.save_borders = (*overscan.borders);

	cfg_oscan.borders = &cfg_oscan.overscan_borders[0];

	/* disabilito gli acceleratori */
	gui.accelerators_anabled = FALSE;

	cfg_oscan.initdialog = TRUE;

	/* disabilito la gestiore del docus della finestra principale */
	gui.main_win_lfp = FALSE;

	emu_pause(TRUE);

	cfg_oscan.father = CreateDialog(GetModuleHandle(NULL ),
			MAKEINTRESOURCE(IDD_OSCAN_BRDS_DIALOG), hwnd, (DLGPROC) cfg_overscan_borders_messages);
}

long __stdcall cfg_overscan_borders_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			cfg_overscan_borders_destory(hwnd, 0);
			return (TRUE);
		case WM_INITDIALOG: {
			int i;
			static struct _ovscan_mode_list_element {
				char name[20];
			} ovscan_mode_list[] = {
				{ "NTSC"      },
				{ "PAL/Dendy" }
			};

			SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_COMBO, CB_RESETCONTENT, 0, 0);

			for (i = 0; i < LENGTH(ovscan_mode_list); i++) {
				SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_COMBO, CB_ADDSTRING, 0,
						(LPARAM) ovscan_mode_list[i].name);
			}

			if (machine.type == NTSC) {
				cfg_oscan.mode = 0;
			} else {
				cfg_oscan.mode = 1;
			}

			SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_COMBO, CB_SETCURSEL, cfg_oscan.mode, 0);

			/* posso utilizzare questo i loop anche per settare gli edit control */
			for (i = 0; i <= IDC_OSCAN_BRDS_SPIN_RIGHT - IDC_OSCAN_BRDS_SPIN_UP; i++) {
				SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_SPIN_UP + i, UDM_SETRANGE,
						OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
				SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_TEXT_UP + i, EM_LIMITTEXT, 2, 0);
			}

			cfg_overscan_borders_update_dialog(hwnd);

			cfg_oscan.initdialog = FALSE;

			return (TRUE);
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK:
					memcpy(&overscan_borders, &cfg_oscan.overscan_borders,
					        sizeof(overscan_borders));
					cfg_overscan_borders_destory(hwnd, IDOK);
					return (TRUE);
				case IDCANCEL:
					cfg_overscan_borders_destory(hwnd, IDCANCEL);
					return (TRUE);
				case IDC_OSCAN_BRDS_COMBO:
					if (HIWORD(wParam) == CBN_SELCHANGE) {
						cfg_oscan.mode = SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_COMBO,
								CB_GETCURSEL, 0, 0);
						cfg_oscan.borders = &cfg_oscan.overscan_borders[cfg_oscan.mode];
						cfg_overscan_borders_update_dialog(hwnd);
					}
					return (TRUE);
				case ID_OSCAN_BRDS_PREVIEW:
					cfg->oscan = OSCAN_ON;
					cfg_oscan.preview = (*cfg_oscan.borders);
					overscan.borders = &cfg_oscan.preview;
					gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
					return (TRUE);
				case ID_OSCAN_BRDS_DEFAULTS:
					cfg_file_set_overscan_default(cfg_oscan.borders, cfg_oscan.mode + NTSC);
					cfg_overscan_borders_update_dialog(hwnd);
					return (TRUE);
			}

			switch (HIWORD(wParam)) {
				case EN_KILLFOCUS: {
					char buffer[5];
					int value = 0;

					{
						WORD *length = (WORD *) &buffer;

						(*length) = sizeof(buffer);
					}

					SendMessage((HWND) lParam, EM_GETLINE, 0, (LPARAM) &buffer);

					value = atoi(buffer);

					if (value > OVERSCAN_BORDERS_MAX) {
						SendDlgItemMessage(hwnd,
								IDC_OSCAN_BRDS_SPIN_UP + (LOWORD(wParam) - IDC_OSCAN_BRDS_TEXT_UP),
								UDM_SETPOS, 0, OVERSCAN_BORDERS_MAX);
					}

					break;
				}
				case EN_UPDATE: {
					char buffer[5];
					BYTE value = 0, *src = (BYTE *) cfg_oscan.borders;

					if (cfg_oscan.initdialog == TRUE) {
						break;
					}

					{
						WORD *length = (WORD *) &buffer;

						(*length) = sizeof(buffer);
					}

					SendMessage((HWND) lParam, EM_GETLINE, 0, (LPARAM) &buffer);

					value = atoi(buffer);
					(*(src + (LOWORD(wParam) - IDC_OSCAN_BRDS_TEXT_UP))) = value;

					break;
				}
			}
			break;
		}
	}
	return (FALSE);
}
void cfg_overscan_borders_update_dialog(HWND hwnd) {
	int i;
	BYTE *src = (BYTE *) cfg_oscan.borders;

	for (i = 0; i <= IDC_OSCAN_BRDS_SPIN_RIGHT - IDC_OSCAN_BRDS_SPIN_UP; i++) {
		BYTE value = (*(src + i));

		SendDlgItemMessage(hwnd, IDC_OSCAN_BRDS_SPIN_UP + i, UDM_SETPOS, 0, value);
	}
}
void cfg_overscan_borders_destory(HWND hwnd, INT_PTR result) {
	EndDialog(hwnd, result);

	/* aggiorno l'attuale tabella */
	cfg_oscan.force_set_mode = overscan_set_mode(machine.type);

	/* ripristino il valore originario del parametro */
	if (cfg_oscan.save_overscan != cfg->oscan) {
		cfg_oscan.force_set_mode = TRUE;
		cfg->oscan = cfg_oscan.save_overscan;
	}

	/*
	 * se le dimensioni dei bordi sono cambiati rispetto ai
	 * valori di ingresso allora forzo il gfx_set_screen.
	 */
	{
		BYTE i, *src = (BYTE *) &cfg_oscan.save_borders, *dst = (BYTE *) overscan.borders;

		for (i = 0; i < sizeof(_overscan_borders); i++) {
			if ((*(src + i)) != (*(dst + i))) {
				cfg_oscan.force_set_mode = TRUE;
				break;
			}
		}
	}

	if (cfg_oscan.force_set_mode == TRUE) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
	}

	emu_pause(FALSE);

	/* riabilito gli acceleratori */
	gui.accelerators_anabled = TRUE;

	/* restituisco alla finestra principale la gestione del focus */
	gui.main_win_lfp = TRUE;
}
