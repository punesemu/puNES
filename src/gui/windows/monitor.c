/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include "video/gfx_monitor.h"

static BYTE add_resolution_to_list(int mode, DEVMODEW *current_settings, DEVMODEW *dm, _monitor_info *mi);
static uint8_t *get_edid(uTCHAR *device_id);

BYTE gui_monitor_enum_monitors(void) {
	DISPLAY_DEVICEW dd, ddm;
	int crtc;

	monitor.enabled = TRUE;

	for (crtc = 0; (memset(&dd, 0x00, sizeof(dd)), dd.cb = sizeof(dd), EnumDisplayDevicesW(0, crtc, &dd, 0)) != 0; crtc++)
	{
		int output;

		for (output = 0;
			(memset(&ddm, 0x00, sizeof(ddm)), ddm.cb = sizeof(ddm), EnumDisplayDevicesW(dd.DeviceName, output, &ddm, 0)) != 0;
			output++)
		{
			int mode;

			if ((ddm.StateFlags & DISPLAY_DEVICE_ACTIVE) && !(ddm.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
				_monitor_info *mi = NULL, *list = NULL;
				DEVMODEW dm, current_settings;

				// cerco la configurazione corrente del monitor
				memset(&current_settings, 0x00, sizeof(current_settings));
				EnumDisplaySettingsW(dd.DeviceName, ENUM_CURRENT_SETTINGS, &current_settings);

				// memorizzo le informazioni necessarie
				list = (_monitor_info *)realloc(monitor.monitors, (monitor.nmonitor + 1) * sizeof(_monitor_info));
				if (!list) {
					return (EXIT_ERROR);
				}
				monitor.monitors = list;
				mi = &monitor.monitors[monitor.nmonitor];
				memset(mi, 0x00, sizeof(_monitor_info));

				ustrncpy(mi->name, dd.DeviceName, usizeof(mi->name) - 1);

				mi->in_use = FALSE;
				mi->rotation = current_settings.dmDisplayOrientation;
				mi->bits_per_pixel = current_settings.dmBitsPerPel;
				mi->fixed_output = current_settings.dmDisplayFixedOutput;
				mi->x = current_settings.dmPosition.x;
				mi->y = current_settings.dmPosition.y;
				mi->w = (int)current_settings.dmPelsWidth;
				mi->h = (int)current_settings.dmPelsHeight;
				mi->mode_org = -1;
				mi->mode_new = -1;
				mi->mode_in_use = -1;

				monitor.nmonitor++;

				for (mode = 0;
					(memset(&dm, 0x00, sizeof(dm)), dm.dmSize = sizeof(dm), EnumDisplaySettingsW(dd.DeviceName, mode, &dm)) != 0;
					mode++)
				{
					if ((mi->bits_per_pixel != dm.dmBitsPerPel) ||
						(mi->fixed_output != dm.dmDisplayFixedOutput) ||
						!dm.dmDisplayFrequency) {
						continue;
					}
					if (add_resolution_to_list(mode, &current_settings, &dm, mi) == EXIT_ERROR) {
						return (EXIT_ERROR);
					}
				}

				// se la risoluzione attuale non e' nella lista di quelle supportate dal monitor
				// la salvo per ultima (VirtualBox con risoluzioni particolari).
				if (mi->mode_org == -1) {
					dm.dmDisplayFlags = current_settings.dmDisplayFlags;
					dm.dmPelsWidth = current_settings.dmPelsWidth;
					dm.dmPelsHeight = current_settings.dmPelsHeight;
					dm.dmDisplayFrequency = current_settings.dmDisplayFrequency;
					if (add_resolution_to_list(mode, &current_settings, &dm, mi) == EXIT_ERROR) {
						return (EXIT_ERROR);
					}
				}

				gfx_monitor_edid_parse(get_edid(ddm.DeviceID), mi);
			}
		}
	}
	return (EXIT_OK);
}
void gui_monitor_set_res(void *monitor_info, void *mode_info) {
	_monitor_info *mi = (_monitor_info *)monitor_info;
	_monitor_mode_info *mmi = (_monitor_mode_info *)mode_info;
	DEVMODEW dm;

	memset(&dm, 0x00, sizeof(dm));

	dm.dmSize = sizeof(dm);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY | DM_DISPLAYFIXEDOUTPUT;
	dm.dmPelsWidth = mmi->w;
	dm.dmPelsHeight = mmi->h;
	dm.dmDisplayFrequency = (DWORD)mmi->rrate;
	dm.dmBitsPerPel = mi->bits_per_pixel;
	dm.dmDisplayFixedOutput = mi->fixed_output;

	ChangeDisplaySettingsExW(mi->name, &dm, NULL, CDS_FULLSCREEN | CDS_UPDATEREGISTRY, NULL);
}
void gui_monitor_get_current_x_y(void *monitor_info, int *x, int *y) {
	_monitor_info *mi = (_monitor_info *)monitor_info;
	DEVMODEW dm;

	memset(&dm, 0x00, sizeof(dm));
	EnumDisplaySettingsW(mi->name, ENUM_CURRENT_SETTINGS, &dm);

	(*x) = dm.dmPosition.x;
	(*y) = dm.dmPosition.y;
}

static BYTE add_resolution_to_list(int mode, DEVMODEW *current_settings, DEVMODEW *dm, _monitor_info *mi) {
	_monitor_mode_info *mm = NULL, *list = NULL;

	list = (_monitor_mode_info *)realloc(mi->modes, (mi->nmodes + 1) * sizeof(_monitor_mode_info));
	if (!list) {
		return (EXIT_ERROR);
	}
	mi->modes = list;
	mm = &mi->modes[mi->nmodes];
	memset(mm, 0x00, sizeof(_monitor_mode_info));

	mm->id = mode;
	mm->flags = dm->dmDisplayFlags;
	mm->w = (int)dm->dmPelsWidth;
	mm->h = (int)dm->dmPelsHeight;
	mm->rrate = dm->dmDisplayFrequency;
	mm->rounded_rrate = dm->dmDisplayFrequency;
	if ((mm->flags == current_settings->dmDisplayFlags) &&
		(mm->w == (int)current_settings->dmPelsWidth) &&
		(mm->h == (int)current_settings->dmPelsHeight) &&
		(mm->rrate == current_settings->dmDisplayFrequency)) {
		mi->mode_org = mi->mode_in_use = mi->nmodes;
	}

	mi->nmodes++;

	return (EXIT_OK);
}
static uint8_t *get_edid(uTCHAR *device_id) {
	uTCHAR *s, model[24], str[MAX_PATH] = uL("SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\");
	uint8_t *pedid = NULL;
	size_t len;
	HKEY hkey;

	memset(model, 0x00, sizeof(model));

	s = ustrchr(device_id, '\\') + 1;
	len = ustrchr(s, '\\') - s;

	if (len >= usizeof(model)) {
		len = usizeof(model) - 1;
	}
	ustrncpy(model, s, len);
	ustrcat(str, model);
	s = ustrchr(s, '\\') + 1;

	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, str, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		DWORD size = MAX_PATH;
		int i = 0;
		FILETIME ft;

		while (RegEnumKeyExW(hkey, i, str, &size, NULL, NULL, NULL, &ft) == ERROR_SUCCESS) {
			HKEY hkey2;

			if (RegOpenKeyExW(hkey, str, 0, KEY_READ, &hkey2) == ERROR_SUCCESS) {
				size = MAX_PATH;
				if (RegQueryValueExW(hkey2, uL("Driver"), NULL, NULL, (LPBYTE)&str, &size) == ERROR_SUCCESS) {
					if (ustrcmp(str, s) == 0) {
						HKEY hkey3;

						if (RegOpenKeyExW(hkey2, uL("Device Parameters"), 0, KEY_READ, &hkey3) == ERROR_SUCCESS) {
							static uint8_t edid[256];

							size = sizeof(edid);
							memset(edid, 0x00, sizeof(edid));

							if (RegQueryValueExW(hkey3, uL("EDID"), NULL, NULL, (LPBYTE)&edid, &size) == ERROR_SUCCESS) {
								if (ustrcmp(model, gfx_monitor_edid_decode_model(edid)) == 0) {
									pedid = edid;
									break;
								}
							}
							RegCloseKey(hkey3);
						}
					}
				}
				RegCloseKey(hkey2);
			}
			i++;
		}
		RegCloseKey(hkey);
	}
	return (pedid);
}
