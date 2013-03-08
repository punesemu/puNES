/*
 * gfx.h
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#ifndef GFX_H_
#define GFX_H_

#include "common.h"
#include "filters/video/scale.h"
#include "filters/video/ntsc.h"

enum scale_type { X1 = 1, X2, X3, X4 };
enum filters_type {
	NO_FILTER,
};

enum overcan_type { OSCAN_OFF, OSCAN_ON, OSCAN_DEFAULT, OSCAN_DEFAULT_OFF, OSCAN_DEFAULT_ON };
enum gfx_info_index { CURRENT, NO_OVERSCAN, MONITOR, VIDEO_MODE };
enum no_change { NO_CHANGE = 255 };

struct _gfx {
	BYTE scale_before_fscreen;
	BYTE bit_per_pixel;
	WORD rows;
	WORD lines;
	SDBWORD w[4];
	SDBWORD h[4];
	float w_pr;
	float h_pr;
} gfx;

BYTE gfx_init(void);
void gfx_set_render(BYTE render);
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale);
void gfx_draw_screen(BYTE forced);
void gfx_reset_video(void);
void gfx_quit(void);

/* funzioni virtuali */
#define GFX_EFFECT_ROUTINE\
	void (*effect)(WORD *screen, WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch,\
			void *pix, WORD rows, WORD lines, BYTE factor)

GFX_EFFECT_ROUTINE;

#endif /* GFX_H_ */
