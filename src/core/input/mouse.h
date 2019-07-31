/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#include "../input.h"
#include "gui.h"
#include "video/gfx.h"

INLINE static void input_read_mouse_coords(int *x, int *y) {
	(*x) = ((float)(gmouse.x - gfx.vp.x) / gfx.w_pr);
	(*y) = ((float)(gmouse.y - gfx.vp.y) / gfx.h_pr);
}

#endif /* INPUT_MOUSE_H_ */
