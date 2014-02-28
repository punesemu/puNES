/*
 * uncompress_selection.c
 *
 *  Created on: 25/dic/2013
 *      Author: fhorse
 */

#if defined (MINGW32)
#define _WIN32_IE 0x0300
#elif defined (MINGW64)
#define _WIN32_IE 0x0501
#endif
#include "win.h"
#include <libgen.h>
#include <stdlib.h>
#include <commctrl.h>
#include "uncompress_selection.h"
#include "uncompress.h"

long __stdcall uncompress_selection_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void uncompress_selection_destroy(HWND hwnd, INT_PTR result);

struct _uncompress_selection_data {
	HWND window;
	int selected;
} uncompress_selection;

int uncompress_selection_dialog(HWND hwnd) {
	memset(&uncompress_selection, 0x00, sizeof(uncompress_selection));

	if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_UNCOMPRESS_SELECTION), hwnd,
	        (DLGPROC) uncompress_selection_messages) == IDCANCEL) {
		return (UNCOMP_NO_FILE_SELECTED);
	}

	return (uncompress_selection.selected);
}

long __stdcall uncompress_selection_messages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			uncompress_selection_destroy(hwnd, IDCANCEL);
			return (TRUE);
		case WM_INITDIALOG: {
			HWND list = GetDlgItem(hwnd, IDC_UNCOMPRESS_SELECTION_LIST);

			SendMessage(list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

			{
				LVCOLUMN column;

				memset(&column, 0x00, sizeof(column));

				column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

				column.cx = 30;
				column.iOrder = 0;
				column.pszText = "ID";
				SendMessage(list, LVM_INSERTCOLUMN, 0, (LPARAM) &column);

				column.cx = 277;
				column.iOrder = 1;
				column.pszText="Roms";
				SendMessage(list, LVM_INSERTCOLUMN, 1, (LPARAM) &column);
			}

			{
				LVITEM item;
				int i;

				memset(&item, 0x00, sizeof(item));

				item.mask = LVIF_TEXT;
				item.cchTextMax = 512;

				for (i = 0; i < uncomp.files_founded; i++) {
					if (uncomp_name_file(&uncomp.file[i]) == EXIT_OK) {
						char count[5];

						item.iItem = i;

						item.iSubItem = 0;
						snprintf(count, sizeof(count), "%d", i);
						item.pszText = (LPTSTR) &count;
						SendMessage(list, LVM_INSERTITEM, 0, (LPARAM) &item);

						item.iSubItem = 1;
						item.pszText = (LPTSTR) basename(uncomp.buffer);
						SendMessage(list, LVM_SETITEM, 0, (LPARAM) &item);
					}
				}

				ListView_SetItemState(list, 0, LVIS_SELECTED , LVIS_SELECTED);
			}

			return (TRUE);
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: {
					HWND list = GetDlgItem(hwnd, IDC_UNCOMPRESS_SELECTION_LIST);
					LVITEM item;
					int index;
					char text[5] = { 0 };

					uncompress_selection_destroy(hwnd, IDOK);

					if ((index = SendMessage(list, LVM_GETNEXTITEM, -1, LVNI_FOCUSED)) == -1) {
						index = 0;
					}

					memset(&item, 0x00, sizeof(item));

					item.mask = LVIF_TEXT;
					item.iSubItem = 0;
					item.pszText = text;
					item.cchTextMax = sizeof(text);
					item.iItem = index;

					SendMessage(list, LVM_GETITEMTEXT, index, (LPARAM) &item);

					uncompress_selection.selected = atoi(text);

					return (TRUE);
				}
				case IDCANCEL:
					uncompress_selection_destroy(hwnd, IDCANCEL);
					return (TRUE);
			}
			break;
		}
	}

	return (FALSE);
}
void uncompress_selection_destroy(HWND hwnd, INT_PTR result) {
	EndDialog(hwnd, result);
}
