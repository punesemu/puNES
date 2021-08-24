/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef GFX_MONITOR_H_
#define GFX_MONITOR_H_

#if !defined (_WIN32)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif
#include "gui.h"
#include "common.h"
#include "unicode_def.h"

typedef struct _monitor_edid_detailed_timing {
	int w_mm, h_mm;
} _monitor_edid_detailed_timing;
typedef struct _monitor_edid {
	int checksum;
	uTCHAR manufacturer_code[4];
	int product_code;
	unsigned int serial_number;

	int w_mm, h_mm;
	double aspect_ratio;

	int ndtimings;
	_monitor_edid_detailed_timing dtimings[4];

	uTCHAR dsc_serial_number[14];
	uTCHAR dsc_product_name[14];
	uTCHAR dsc_string[14];
} _monitor_edid;
typedef struct _monitor_mode_info {
	int w, h;
	double rrate;
	double rounded_rrate;
#if defined (_WIN32)
	int id;
	DWORD flags;
#else
	RRMode id;
	XRRModeFlags flags;
#endif
} _monitor_mode_info;
typedef struct _monitor_info {
	BYTE in_use;
	uTCHAR name[64];
	uTCHAR desc[64];
	int x, y;
	int w, h;
	int nmodes;
	_monitor_mode_info *modes;
	int mode_org;
	int mode_new;
	int mode_in_use;
	_monitor_edid *edid;
#if defined (_WIN32)
	DWORD rotation;
	DWORD bits_per_pixel;
	DWORD fixed_output;
#else
	RRCrtc crtc;
	Rotation rotation;
	RROutput output;
#endif
} _monitor_info;
typedef struct _monitor_resolution {
	int w, h;
} _monitor_resolution;
typedef struct _monitor {
	BYTE enabled;
	int nmonitor;
	_monitor_info *monitors;
	int active;
	int nres;
	_monitor_resolution *resolutions;
} _monitor;

extern _monitor monitor;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void gfx_monitor_init(void);
EXTERNC void gfx_monitor_quit(void);
EXTERNC void gfx_monitor_enum_monitors(void);
EXTERNC void gfx_monitor_enum_resolutions(void);
EXTERNC BYTE gfx_monitor_set_res(int w, int h, BYTE adaptive_rrate, BYTE change_rom_mode);
EXTERNC BYTE gfx_monitor_restore_res(void);
EXTERNC void gfx_monitor_mode_in_use_info(int *x, int *y, int *w, int *h, int *rrate);
EXTERNC void gfx_monitor_edid_parse(const uint8_t *edid, _monitor_info *mi);
EXTERNC uTCHAR *gfx_monitor_edid_decode_model(const uint8_t *edid);

#undef EXTERNC

#endif /* GFX_MONITOR_H_ */
