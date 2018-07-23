/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef GFX_FUNCTIONS_INLINE_H_
#define GFX_FUNCTIONS_INLINE_H_

#include "common.h"

#if defined (__GFX_FORCE_SCALE__) || defined (__GFX_ALL_FUNC__)
static void INLINE gfx_FORCE_SCALE(void);
static void INLINE gfx_FORCE_SCALE(void) {
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
#endif

#if defined (__GFX_OTHERS_FUNC__) || defined (__GFX_ALL_FUNC__)
static void INLINE gfx_MAKE_RESET(int type);
static void INLINE gfx_CHANGE_ROM(void);
static void INLINE gfx_SWITCH_MODE(void);
static void INLINE gfx_SCALE(int scale);
static void INLINE gfx_FILTER(int filter);
static void INLINE gfx_SHADER(int shader);
static void INLINE gfx_VSYNC(void);

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
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, TRUE);
}
static void INLINE gfx_SCALE(int scale) {
	gfx_set_screen(scale, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
}
static void INLINE gfx_FILTER(int filter) {
	gfx_set_screen(NO_CHANGE, filter, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	if (cfg->filter == NTSC_FILTER) {
		ntsc_set(NULL, cfg->ntsc_format, 0, 0, (BYTE *) palette_RGB, 0);
	}
}
static void INLINE gfx_SHADER(int shader) {
	gfx_set_screen(NO_CHANGE, NO_CHANGE, shader, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
}
static void INLINE gfx_VSYNC(void) {
#if defined (WITH_OPENGL)
	sdl_wid();
	gfx_reset_video();
#endif
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
#endif

#endif /* GFX_FUNCTIONS_INLINE_H_ */
