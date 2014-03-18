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

#include <SDL.h>
#include "common.h"
#include "text.h"
#include "filters/video/scale.h"
#include "filters/video/scale2x.h"
#include "filters/video/hqx.h"
#include "filters/video/bilinear.h"
#include "filters/video/ntsc.h"

enum render_type { RENDER_SOFTWARE, RENDER_OPENGL, RENDER_GLSL };
enum scale_type { X1 = 1, X2, X3, X4 };
enum filters_type {
	NO_FILTER,
	SCALE2X,
	SCALE3X,
	SCALE4X,
	HQ2X,
	HQ3X,
	HQ4X,
	NTSC_FILTER,
	BILINEAR,
	/* glsl shaders */
	POSPHOR,
	SCANLINE,
	DBL,
	CRT_CURVE,
	CRT_NO_CURVE,
	/* glsl shaders end */
};
enum fullscreen_type { NO_FULLSCR, FULLSCR };
enum overcan_type { OSCAN_OFF, OSCAN_ON, OSCAN_DEFAULT, OSCAN_DEFAULT_OFF, OSCAN_DEFAULT_ON };
enum gfx_info_type { CURRENT, NO_OVERSCAN, MONITOR, VIDEO_MODE };
enum no_change { NO_CHANGE = 255 };

struct _gfx {
	BYTE scale_before_fscreen;
	BYTE opengl;
	BYTE bit_per_pixel;
	WORD rows;
	WORD lines;
	SDBWORD w[4];
	SDBWORD h[4];
	float w_pr;
	float h_pr;
	float aspect_ratio;
	gfx_filter_function((*filter));
} gfx;

SDL_Surface *surface_sdl;

BYTE gfx_init(void);
void gfx_set_render(BYTE render);
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale);
void gfx_draw_screen(BYTE forced);
void gfx_reset_video(void);
void gfx_quit(void);

void gfx_text_create_surface(_txt_element *ele);
void gfx_text_release_surface(_txt_element *ele);
void gfx_text_rect_fill(_txt_element *ele, _rect *rect, uint32_t color);
void gfx_text_reset(void);
void gfx_text_clear(_txt_element *ele);
void gfx_text_blit(_txt_element *ele, _rect *rect);

SDL_Surface *gfx_create_RGB_surface(SDL_Surface *src, uint32_t width, uint32_t height);

double sdl_get_ms(void);

int (*flip)(SDL_Surface *surface);

#endif /* GFX_H_ */
