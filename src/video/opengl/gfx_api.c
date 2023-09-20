/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *  for some codes :
 *  Copyright (C) 2010-2015 The RetroArch team
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
 *
 */

#include "video/gfx.h"
#include "opengl.h"
#include "gui.h"
#include "ppu.h"

#if defined (_WIN32)
HMONITOR monitor_in_use;
#endif

BYTE gfx_api_init(void) {
#if defined (_WIN32)
	monitor_in_use = MonitorFromWindow(gui_win_id(), MONITOR_DEFAULTTOPRIMARY);
#endif
	return (opengl_init());
}
void gfx_api_quit(void) {
	opengl_quit();
}
BYTE gfx_api_context_create(void) {
	return (opengl_context_create());
}
uint32_t gfx_api_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (gui_color(a, r, g, b));
}
void gfx_api_overlay_blit(void *surface, _gfx_rect *rect, double device_pixel_ratio) {
	if (((rect->x + rect->w) > (float)opengl.overlay.rect.w) || ((rect->y + rect->h) > (float)opengl.overlay.rect.h)) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, opengl.overlay.id);
	glPixelStoref(GL_UNPACK_ROW_LENGTH, rect->w * (float)device_pixel_ratio);
	glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)rect->x, (GLint)rect->y, (GLsizei)rect->w, (GLsizei)rect->h, TI_FRM, TI_TYPE, surface);
	glPixelStoref(GL_UNPACK_ROW_LENGTH, 0);
}
void gfx_api_apply_filter(void) {
	gfx.frame.filtered = ppudata.ppu_screen.rd->frame;

	// applico l'effetto desiderato
	gfx.filter.data.pitch = opengl.surface.pitch;
	gfx.filter.data.pix = opengl.surface.pixels;
	gfx.filter.data.width = opengl.surface.w;
	gfx.filter.data.height = opengl.surface.h;
	gfx.filter.func();
}
void gfx_api_control_changed_adapter(UNUSED(void *monitor)) {
#if defined (_WIN32)
	HMONITOR *in_use = monitor;

	if ((*in_use) == monitor_in_use) {
		return;
	}
	monitor_in_use = (*in_use);
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
#endif
}
