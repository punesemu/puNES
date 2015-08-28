/*
 * gfx_functions_inline.h
 *
 *  Created on: 28 ago 2015
 *      Author: fhorse
 */

#ifndef GFX_FUNCTIONS_INLINE_H_
#define GFX_FUNCTIONS_INLINE_H_

#include "common.h"

#if defined (__GFX_FORCE_SCALE__) || defined (__GFX_ALL_FUNC__)
static void INLINE gfx_FORCE_SCALE(void);
static void INLINE gfx_FORCE_SCALE(void) {
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
#endif

#if defined (__GFX_OTHERS_FUNC__) || defined (__GFX_ALL_FUNC__)
static void INLINE gfx_SWITCH_RENDERING(void);
static void INLINE gfx_MAKE_RESET(int type);
static void INLINE gfx_CHANGE_ROM(void);
static void INLINE gfx_SWITCH_MODE(void);
static void INLINE gfx_SCALE(int scale);
static void INLINE gfx_FILTER(int filter);
static void INLINE gfx_VSYNC(void);

static void INLINE gfx_SWITCH_RENDERING(void) {
#if defined (SDL)
	sdl_wid();
	opengl_effect_change(opengl.rotation);
	gfx_reset_video();
#endif
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
static void INLINE gfx_MAKE_RESET(int type) {
	gui_mainWindow_make_reset(type);
}
static void INLINE gfx_CHANGE_ROM(void) {
	gui_mainWindow_make_reset(CHANGE_ROM);
	gui_update();
}
static void INLINE gfx_SWITCH_MODE(void) {
	gui_mainWindow_make_reset(CHANGE_MODE);
	/*
	 * per lo swap dell'emphasis del rosso e del verde in caso di PAL e DENDY
	 * ricreo la paletta quando cambio regione.
	 */
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, TRUE);
}
static void INLINE gfx_SCALE(int scale) {
	gfx_set_screen(scale, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
}
static void INLINE gfx_FILTER(int filter) {
	switch (filter) {
		case NO_FILTER:
			gfx_set_screen(NO_CHANGE, NO_FILTER, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case PHOSPHOR:
			gfx_set_screen(NO_CHANGE, PHOSPHOR, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case PHOSPHOR2:
			gfx_set_screen(NO_CHANGE, PHOSPHOR2, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case SCANLINE:
			gfx_set_screen(NO_CHANGE, SCANLINE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case DBL:
			gfx_set_screen(NO_CHANGE, DBL, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case DARK_ROOM:
			gfx_set_screen(NO_CHANGE, DARK_ROOM, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case CRT_CURVE:
			gfx_set_screen(NO_CHANGE, CRT_CURVE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case CRT_NO_CURVE:
			gfx_set_screen(NO_CHANGE, CRT_NO_CURVE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case SCALE2X:
			gfx_set_screen(X2, SCALE2X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case SCALE3X:
			gfx_set_screen(X3, SCALE3X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case SCALE4X:
			gfx_set_screen(X4, SCALE4X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case HQ2X:
			gfx_set_screen(X2, HQ2X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case HQ3X:
			gfx_set_screen(X3, HQ3X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case HQ4X:
			gfx_set_screen(X4, HQ4X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case XBRZ2X:
			gfx_set_screen(X2, XBRZ2X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case XBRZ3X:
			gfx_set_screen(X3, XBRZ3X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case XBRZ4X:
			gfx_set_screen(X4, XBRZ4X, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			break;
		case NTSC_FILTER:
			gfx_set_screen(NO_CHANGE, NTSC_FILTER, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			if (cfg->filter == NTSC_FILTER) {
				ntsc_set(cfg->ntsc_format, 0, 0, (BYTE *) palette_RGB, 0);
			}
			break;
	}
}
static void INLINE gfx_VSYNC(void) {
#if defined (SDL)
	sdl_wid();
	gfx_reset_video();
#endif
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
#endif

#endif /* GFX_FUNCTIONS_INLINE_H_ */
