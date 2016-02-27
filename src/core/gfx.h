/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
	void name(WORD *screen, WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch,\
	void *pix, WORD rows, WORD lines, WORD width, WORD height, BYTE factor)
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

enum fullscreen_type { NO_FULLSCR, FULLSCR };
enum scale_type { X1 = 1, X2, X3, X4 };
enum par_type { PAR11, PAR54, PAR87 };
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
	FUTURE_USE0,
	FUTURE_USE1,
	FUTURE_USE2,
	FUTURE_USE3,
	FUTURE_USE4,
	FUTURE_USE5,
	FUTURE_USE6,
	FUTURE_USE7,
	FUTURE_USE8,
	FUTURE_USE9,
	FUTURE_USE10,
	FUTURE_USE11,
	FUTURE_USE12,
	FUTURE_USE13,
	FUTURE_USE14,
	/* shaders */
	FLTSHDSTART,
	sh_anti_aliasing_advanced_aa = FLTSHDSTART,
	sh_anti_aliasing_fx_aa,
	sh_anti_aliasing_fxaa_edge_detect,
	sh_cgp_tvout_tvout_ntsc_2phase_composite,
	sh_cgp_tvout_tvout_ntsc_256px_svideo,
	sh_cgp_2xbr_crt_hyllian,
	sh_cgp_2xbr_jinc2_sharper_hybrid,
	sh_crt_gtuv50,
	sh_crt_4xbr_hybrid_crt,
	sh_crt_crt_caligari,
	sh_crt_crt_cgwg_fast,
	sh_crt_crt_easymode,
	sh_crt_crt_easymode_halation,
	sh_crt_crt_geom,
	sh_crt_crtglow_gauss,
	sh_crt_crtglow_gauss_ntsc_3phase,
	sh_crt_crt_hyllian,
	sh_crt_crt_lottes,
	sh_crt_crt_reverse_aa,
	sh_crt_dotmask,
	sh_eagle_super_eagle,
	sh_hunterk_borders_1080p_bigblur,
	sh_hunterk_borders_1080p_color_grid,
	sh_hunterk_borders_1080p_mudlord,
	sh_hunterk_borders_1080p_shiny_iterations,
	sh_hunterk_borders_1080p_snow,
	sh_hunterk_borders_1080p_voronoi,
	sh_hunterk_borders_1080p_water,
	sh_hunterk_handheld_nds,
	sh_hunterk_hqx_hq3x,
	sh_hunterk_motionblur_motionblur_simple,
	sh_motionblur_feedback,
	sh_mudlord_emboss,
	sh_mudlord_mud_mudlord,
	sh_mudlord_noise_mudlord,
	sh_mudlord_oldtv,
	sh_waterpaint_water,

	sh_test,
	FLTSHDSTOP = sh_waterpaint_water
	/* shaders end */
};
enum overcan_type { OSCAN_OFF, OSCAN_ON, OSCAN_DEFAULT, OSCAN_DEFAULT_OFF, OSCAN_DEFAULT_ON };
enum gfx_info_type { CURRENT, NO_OVERSCAN, MONITOR, VIDEO_MODE };
enum no_change { NO_CHANGE = 255 };

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#if defined (SDL)
#include <SDL.h>

enum render_type { RENDER_SOFTWARE, RENDER_GLSL };
#if defined (__WIN32__)
enum sdl_win_event_type {
	SDLWIN_NONE,
	SDLWIN_SWITCH_RENDERING,
	SDLWIN_MAKE_RESET,
	SDLWIN_CHANGE_ROM,
	SDLWIN_SWITCH_MODE,
	SDLWIN_FORCE_SCALE,
	SDLWIN_SCALE,
	SDLWIN_FILTER,
	SDLWIN_VSYNC
};

EXTERNC struct _sdlwe {
	int event;
	int arg;
} sdlwe;
#endif

EXTERNC SDL_Surface *surface_sdl;

EXTERNC void gfx_reset_video(void);

EXTERNC SDL_Surface *gfx_create_RGB_surface(SDL_Surface *src, uint32_t width, uint32_t height);
EXTERNC double sdl_get_ms(void);
EXTERNC int (*flip)(SDL_Surface *surface);
#elif defined (D3D9)
enum render_type { RENDER_SOFTWARE, RENDER_HLSL };

typedef struct _texcoords {
	float l, r;
	float t, b;
} _texcoords;

EXTERNC void gfx_control_change_monitor(void *monitor);
#endif

EXTERNC struct _gfx {
	BYTE scale_before_fscreen;
	BYTE bit_per_pixel;
	WORD rows;
	WORD lines;
	SDBWORD w[4];
	SDBWORD h[4];
	float w_pr;
	float h_pr;
	float pixel_aspect_ratio;
	gfx_filter_function((*filter));
#if defined (SDL)
	BYTE opengl;
#elif defined (D3D9)
	struct {
		BYTE compliant;
		BYTE enabled;
		BYTE used;
		BYTE param;
	} hlsl;
	_texcoords quadcoords;
#endif
} gfx;

EXTERNC BYTE gfx_init(void);
EXTERNC void gfx_set_render(BYTE render);
EXTERNC void gfx_set_screen(BYTE scale, DBWORD filter, BYTE fullscreen, BYTE palette,
		BYTE force_scale, BYTE force_palette);

EXTERNC void gfx_draw_screen(BYTE forced);
EXTERNC void gfx_quit(void);

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

#if defined (SDL) && defined (__WIN32__)
EXTERNC void gfx_sdlwe_set(int type, int arg);
EXTERNC void gfx_sdlwe_tick(void);
#endif

#undef EXTERNC

#endif /* GFX_H_ */
