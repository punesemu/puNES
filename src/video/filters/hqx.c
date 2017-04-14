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

#include "video/filters/hqx.h"
#include "overscan.h"

uint32_t RGBtoYUV[NUM_COLORS];
uint32_t YUV1, YUV2;

void hqx_init(void) {
	/* Initalize RGB to YUV lookup table */
	uint32_t i, r, g, b, y, u, v;

	for (i = 0; i < NUM_COLORS; i++) {
		r = palette_RGB[i].r;
		g = palette_RGB[i].g;
		b = palette_RGB[i].b;
		y = (uint32_t) (0.299 * r + 0.587 * g + 0.114 * b);
		u = (uint32_t) (-0.169 * r - 0.331 * g + 0.5 * b) + 128;
		v = (uint32_t) (0.5 * r - 0.419 * g - 0.081 * b) + 128;
		RGBtoYUV[i] = (y << 16) + (u << 8) + v;
	}
}
gfx_filter_function(hqNx) {
	hqnx.sx = 0;
	hqnx.sy = 0;
	hqnx.lines = SCR_LINES;
	hqnx.rows = SCR_ROWS;
	hqnx.dst_rows = width / factor;
	hqnx.startx = 0;

	if (factor == 1) {
		return;
	} else if (factor == 2) {
		hq2x_32_rb(screen, pix, (uint32_t *) palette);
	} else if (factor == 3) {
		hq3x_32_rb(screen, pix, (uint32_t *) palette);
	} else if (factor == 4) {
		hq4x_32_rb(screen, pix, (uint32_t *) palette);
	}
}
