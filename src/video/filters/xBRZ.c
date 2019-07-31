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

#include "video/filters/xBRZ.h"
#include "ppu.h"

void xBRZ_init(void) {}
void xBRZ(void) {
	xbrz_scale(gfx.filter.factor, screen.rd->data, (uint32_t *)gfx.filter.data.pix, (uint32_t *)gfx.filter.data.palette,
		SCR_ROWS, SCR_LINES);
}
void xBRZ_mt(void) {
	xbrz_scale_mt(gfx.filter.factor, screen.rd->data, (uint32_t *)gfx.filter.data.pix, (uint32_t *)gfx.filter.data.palette,
		SCR_ROWS, SCR_LINES);
}
