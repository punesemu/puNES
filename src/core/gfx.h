/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

/* definizione funzione virtuale */
#if !defined (gfx_filter_function)
#define gfx_filter_function(name)\
	void name(WORD *screen, WORD **screen_index, void *palette, uint32_t pitch, void *pix,\
	WORD width, WORD height)
#endif

#ifndef GFX_H_
#define GFX_H_

#include "common.h"
#include "text.h"
#include "video/filters/scale.h"
#include "video/filters/scale2x.h"
#include "video/filters/hqx.h"
#include "video/filters/ntsc.h"
#include "video/filters/xBRZ.h"
#if defined (WITH_OPENGL)
#include <SDL.h>
#endif

#define FH_SHADERS_GEST
#define change_color(plt, blck, index, color, operation)\
	tmp = plt[index].color + operation;\
	plt[index].color = (tmp < 0 ? blck : (tmp > 0xFF ? 0xFF : tmp))
#define rgb_modifier(ntscin, plt, blck, red, green, blue)\
	/* prima ottengo la paletta monocromatica */\
	ntsc_set(ntscin, cfg->ntsc_format, PALETTE_MONO, 0, 0, (BYTE *) plt);\
	/* quindi la modifico */\
	{\
		WORD i;\
		SWORD tmp;\
		for (i = 0; i < NUM_COLORS; i++) {\
			/* rosso */\
			change_color(plt, blck, i, r, red);\
			/* green */\
			change_color(plt, blck, i, g, green);\
			/* blue */\
			change_color(plt, blck, i, b, blue);\
		}\
	}\
	/* ed infine utilizzo la nuova */\
	ntsc_set(ntscin, cfg->ntsc_format, FALSE, 0, (BYTE *) plt,(BYTE *) plt)

enum fullscreen_type { NO_FULLSCR, FULLSCR, FULLSCR_IN_WINDOW };
enum scale_type { X1 = 1, X2, X3, X4, X5, X6 };
enum par_type { PAR11, PAR54, PAR87, PAR118 };
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

enum overcan_type { OSCAN_OFF, OSCAN_ON, OSCAN_DEFAULT, OSCAN_DEFAULT_OFF, OSCAN_DEFAULT_ON };
enum gfx_info_type { CURRENT, NO_OVERSCAN, MONITOR, VIDEO_MODE, PASS0 };
enum no_change { NO_CHANGE = 255 };
#if defined (WITH_OPENGL) && defined (__WIN32__)
enum sdl_win_event_type {
	SDLWIN_NONE,
	SDLWIN_MAKE_RESET,
	SDLWIN_CHANGE_ROM,
	SDLWIN_SWITCH_MODE,
	SDLWIN_FORCE_SCALE,
	SDLWIN_SCALE,
	SDLWIN_FILTER,
	SDLWIN_SHADER,
	SDLWIN_VSYNC
};
#endif

typedef struct _viewport {
	int x, y;
	int w, h;
} _viewport;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _gfx {
	BYTE PSS;
	BYTE save_screenshot;
	BYTE scale_before_fscreen;
	BYTE type_of_fscreen_in_use;
	BYTE bit_per_pixel;
	float width_pixel;
	WORD rows, lines;
	SDBWORD w[5], h[5];
	float w_pr, h_pr;
	float pixel_aspect_ratio;
	_viewport vp;

	uint32_t *palette;

	struct _gfx_filter {
		gfx_filter_function((*func));
		BYTE factor;
		float width_pixel;
	} filter;

	uTCHAR last_shader_file[LENGTH_FILE_NAME_LONG];
} gfx;

#if defined (WITH_OPENGL)
EXTERNC SDL_Surface *surface_sdl;

EXTERNC void gfx_reset_video(void);

EXTERNC SDL_Surface *gfx_create_RGB_surface(SDL_Surface *src, uint32_t width, uint32_t height);
EXTERNC double sdl_get_ms(void);

#if defined (__WIN32__)
EXTERNC struct _sdlwe {
	int event;
	int arg;
} sdlwe;

EXTERNC void gfx_sdlwe_set(int type, int arg);
EXTERNC void gfx_sdlwe_tick(void);
#endif
#elif defined (WITH_D3D9)
EXTERNC void gfx_control_changed_adapter(void *monitor);
#endif

EXTERNC BYTE gfx_init(void);

EXTERNC void gfx_set_screen(BYTE scale, DBWORD filter, DBWORD shader, BYTE fullscreen, BYTE palette,
		BYTE force_scale, BYTE force_palette);

EXTERNC void gfx_draw_screen(BYTE forced);
EXTERNC void gfx_quit(void);

EXTERNC uint32_t gfx_color(BYTE alpha, BYTE r, BYTE g, BYTE b);

EXTERNC void gfx_cursor_init(void);
EXTERNC void gfx_cursor_quit(void);
EXTERNC void gfx_cursor_set(void);
#if defined (__linux__)
EXTERNC void gfx_cursor_hide(BYTE hide);
#endif

EXTERNC void gfx_text_create_surface(_txt_element *ele);
EXTERNC void gfx_text_release_surface(_txt_element *ele);
EXTERNC void gfx_text_rect_fill(_txt_element *ele, _rect *rect, uint32_t color);
EXTERNC void gfx_text_reset(void);
EXTERNC void gfx_text_clear(_txt_element *ele);
EXTERNC void gfx_text_blit(_txt_element *ele, _rect *rect);

#undef EXTERNC

#endif /* GFX_H_ */
