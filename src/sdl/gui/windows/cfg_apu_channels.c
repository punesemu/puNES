/*
 * cfg_apu_channels.c
 *
 *  Created on: 14/nov/2013
 *      Author: fhorse
 */

#include "win.h"
#include <commctrl.h>
#include "cfg_apu_channels.h"
#include "cfg_file.h"

long __stdcall apu_channels_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void apu_channels_check(HWND hwnd);
void apu_channels_toggle_all(HWND hwnd, int mode);

struct _apu_channels_data {
	HWND toplevel;
	_config_apu cfg_save;
} apu_channels_data;

void apu_channels_dialog(HWND hwnd) {
	memset(&apu_channels_data, 0x00, sizeof(apu_channels_data));
	memcpy(&apu_channels_data.cfg_save, &cfg->apu, sizeof(_config_apu));

	apu_channels_data.toplevel = CreateDialog(GetModuleHandle(NULL),
	        MAKEINTRESOURCE(IDD_APU_CHANNELS), hwnd, (DLGPROC) apu_channels_wnd_proc);

	SetFocus(apu_channels_data.toplevel);
}
long __stdcall apu_channels_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
			apu_channels_check(hwnd);
			return (TRUE);
		case WM_HSCROLL: {
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			int channel = ctrlID - IDC_APU_CHANNELS_SQUARE1_SLIDER;
			int value = SendDlgItemMessage(hwnd, ctrlID, TBM_GETPOS, 0, 0);

			cfg->apu.volume[channel] = ((double) value ) / 100.0f;

			return (TRUE);
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_APU_CHANNELS_SQUARE1_CHECKBOX:
				case IDC_APU_CHANNELS_SQUARE2_CHECKBOX:
				case IDC_APU_CHANNELS_TRIANGLE_CHECKBOX:
				case IDC_APU_CHANNELS_NOISE_CHECKBOX:
				case IDC_APU_CHANNELS_DMC_CHECKBOX:
				case IDC_APU_CHANNELS_EXTRA_CHECKBOX: {
					int channel = LOWORD(wParam) - IDC_APU_CHANNELS_SQUARE1_CHECKBOX;

					cfg->apu.channel[channel] = !cfg->apu.channel[channel];
					return (TRUE);
				}
				case IDC_APU_CHANNELS_ACTIVE_ALL_BUTTON:
					apu_channels_toggle_all(hwnd, TRUE);
					return (TRUE);
				case IDC_APU_CHANNELS_DISABLE_ALL_BUTTON:
					apu_channels_toggle_all(hwnd, FALSE);
					return (TRUE);
				case IDC_APU_CHANNELS_DEFAULTS_BUTTON:
					apu_channels_toggle_all(hwnd, 2);
					return (TRUE);
				case ID_APU_CHANNELS_OK:
					EndDialog(hwnd, ID_APU_CHANNELS_OK);
					return (TRUE);
					break;
				case ID_APU_CHANNELS_CANCEL:
					memcpy(&cfg->apu, &apu_channels_data.cfg_save, sizeof(_config_apu));
					EndDialog(hwnd, ID_APU_CHANNELS_CANCEL);
					return (TRUE);
			}
			break;
	}

	return (FALSE);
}
void apu_channels_check(HWND hwnd) {
	int i;

	for (i = IDC_APU_CHANNELS_SQUARE1_CHECKBOX; i <= IDC_APU_CHANNELS_EXTRA_CHECKBOX; i++) {
		int status = cfg->apu.channel[i - IDC_APU_CHANNELS_SQUARE1_CHECKBOX];

		if (status == TRUE) {
			SendDlgItemMessage(hwnd, i, BM_SETCHECK, (WPARAM) BST_CHECKED, 0);
		} else {
			SendDlgItemMessage(hwnd, i, BM_SETCHECK, (WPARAM) BST_UNCHECKED, 0);
		}
	}

	for (i = IDC_APU_CHANNELS_SQUARE1_SLIDER; i <= IDC_APU_CHANNELS_MASTER_SLIDER; i++) {
		int pos = cfg->apu.volume[i - IDC_APU_CHANNELS_SQUARE1_SLIDER] * 100;

		SendDlgItemMessage(hwnd, i, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
		SendDlgItemMessage(hwnd, i, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) pos);
	}
}
void apu_channels_toggle_all(HWND hwnd, int mode) {
	int index;

	if (mode == 2) {
		for (index = APU_S1; index <= APU_MASTER; index++) {
			cfg->apu.volume[index] = 1.0f;
		}
		mode = TRUE;
	}
	/*
	 * non devo forzare cfg->apu.channel[APU_MASTER] perche'
	 * lo utilizzo per abilitare o disabilitare il suono
	 * globalmente e viene impostato altrove.
	 */
	for (index = APU_S1; index <= APU_EXTRA; index++) {
		cfg->apu.channel[index] = mode;
	}
	apu_channels_check(hwnd);
}
