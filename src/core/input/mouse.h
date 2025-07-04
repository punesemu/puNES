/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef INPUT_MOUSE_H_
#define INPUT_MOUSE_H_

#include "gui.h"
#include "video/gfx.h"
#include "conf.h"

INLINE static void input_read_mouse_coords(int *x, int *y) {
	double dpr = gui_device_pixel_ratio();
	int mx, my;

	mx = (int)(((float)gmouse.x - (gfx.vp.x / dpr)) / gfx.w_pr);
	my = (int)(((float)gmouse.y - (gfx.vp.y / dpr)) / gfx.h_pr);

	switch (cfg->screen_rotation) {
		default:
		case ROTATE_0:
			if (cfg->hflip_screen) {
				(*x) = SCR_COLUMNS - mx;
			} else {
				(*x) = mx;
			}
			(*y) = my;
			break;
		case ROTATE_90:
			if (cfg->hflip_screen) {
				(*x) = SCR_COLUMNS - my;
			} else {
				(*x) = my;
			}
			(*y) = SCR_ROWS - mx;
			break;
		case ROTATE_180:
			if (cfg->hflip_screen) {
				(*x) = mx;
			} else {
				(*x) = SCR_COLUMNS - mx;
			}
			(*y) = SCR_ROWS - my;
			break;
		case ROTATE_270:
			if (cfg->hflip_screen) {
				(*x) = my;
			} else {
				(*x) = SCR_COLUMNS - my;
			}
			(*y) = mx;
			break;
	}
}

#endif /* INPUT_MOUSE_H_ */
