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

#ifndef GFX_H_
#define GFX_H_

#include "common.h"
#include "video/filters/scale.h"
#include "video/filters/scale2x.h"
#include "video/filters/hqx.h"
#include "video/filters/ntsc.h"
#include "video/filters/xBRZ.h"

#define FH_SHADERS_GEST
#if defined (__unix__)
#define	gfx_os_color(r, g, b) gfx_color(0, r, g, b);
#else
#define	gfx_os_color(r, g, b) gfx_color(255, r, g, b);
#endif

enum fullscreen_type { NO_FULLSCR, FULLSCR, FULLSCR_IN_WINDOW };
enum scale_type { X1 = 1, X2, X3, X4, X5, X6 };
enum par_type { PAR11, PAR54, PAR87, PAR118 };
enum screenshot_type { SCRSH_NONE, SCRSH_STANDARD, SCRSH_ORIGINAL_SIZE };
enum filters_type {
	NO_FILTER,
	SCALE2X,
	SCALE3X,
	SCALE4X,
	HQ2X,
	HQ3X,
	HQ4X,
	NTSC_FILTER,
	XBRZ2X,
	XBRZ3X,
	XBRZ4X,
	XBRZ5X,
	XBRZ6X,
	XBRZ2XMT,
	XBRZ3XMT,
	XBRZ4XMT,
	XBRZ5XMT,
	XBRZ6XMT
};
enum shader_type {
	NO_SHADER,
	SHADER_CRTDOTMASK,
	SHADER_CRTSCANLINES,
	SHADER_CRTWITHCURVE,
	SHADER_EMBOSS,
	SHADER_NOISE,
	SHADER_NTSC2PHASECOMPOSITE,
	SHADER_OLDTV,
	SHADER_FILE,
	SHADER_LAST = SHADER_FILE,
};
enum overcan_type { OSCAN_OFF, OSCAN_ON, OSCAN_DEFAULT };
enum gfx_info_type { CURRENT, NO_OVERSCAN, MONITOR, VIDEO_MODE, PASS0 };
enum no_change { NO_CHANGE = 255 };
enum gfx_rotate_type { ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270, ROTATE_MAX };

typedef struct _gfx_rect {
	float x, y;
	float w, h;
} _gfx_rect;
typedef struct _viewport {
	int x, y;
	int w, h;
} _viewport;
typedef struct _gfx {
	BYTE PSS;
	BYTE scale_before_fscreen;
	BYTE type_of_fscreen_in_use;
	BYTE bit_per_pixel;
	float width_pixel;
	WORD rows, lines;
	SDBWORD w[5], h[5];
	float w_pr, h_pr;
	float pixel_aspect_ratio;
	float device_pixel_ratio;
	uint32_t *palette;
	uTCHAR last_shader_file[LENGTH_FILE_NAME_LONG];
	_viewport vp;
	struct _gfx_frame {
		uint64_t totals;
		uint64_t filtered;
		uint64_t in_draw;
	} frame;
	struct _gfx_filter {
		void (*func)(void);

		float width_pixel;
		BYTE factor;

		struct _gfx_filter_data {
			void *palette;
			uint32_t pitch;
			void *pix;
			WORD width;
			WORD height;
		} data;
	} filter;
} _gfx;

extern _gfx gfx;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE gfx_init(void);
EXTERNC void gfx_quit(void);
EXTERNC BYTE gfx_palette_init(void);
EXTERNC void gfx_set_screen(BYTE scale, DBWORD filter, DBWORD shader, BYTE fullscreen, BYTE palette, BYTE force_scale, BYTE force_palette);
EXTERNC void gfx_draw_screen(void);

#if defined (WITH_D3D9)
EXTERNC void gfx_control_changed_adapter(void *monitor);
#endif

EXTERNC uint32_t gfx_color(BYTE alpha, BYTE r, BYTE g, BYTE b);
EXTERNC void gfx_palette_update(void);

EXTERNC void gfx_cursor_init(void);
EXTERNC void gfx_cursor_set(void);

EXTERNC void gfx_overlay_blit(void *surface, _gfx_rect *rect);

EXTERNC void gfx_apply_filter(void);

#undef EXTERNC

#endif /* GFX_H_ */
