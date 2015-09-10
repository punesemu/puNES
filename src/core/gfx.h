/*
 * gfx.h
 *
 *  Created on: 07/apr/2010
 *      Author: fhorse
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
	/* shaders */
	PHOSPHOR,
	SCANLINE,
	DBL,
	CRT_CURVE,
	CRT_NO_CURVE,
	PHOSPHOR2,
	DARK_ROOM,
	/* shaders end */
	XBRZ2X,
	XBRZ3X,
	XBRZ4X
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

enum render_type { RENDER_SOFTWARE, RENDER_OPENGL, RENDER_GLSL };
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

EXTERNC struct {
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

typedef struct {
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
EXTERNC void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale,
		BYTE force_palette);

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
