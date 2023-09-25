/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include "video/gfx.h"
#include "ppu.h"
#include "info.h"

#define put_pixel(type, p0, p1) *(type *)(dstpix + (p0) + (p1)) = (type)pixel

INLINE static void scale_surface1x(_ppu_screen_buffer *sb, const uint32_t *palette, uint32_t pitch, void *pix);

struct _scl {
	WORD sx;
	WORD sy;
	WORD oy;
	WORD ox;
	WORD startx;
	WORD rows;
	WORD lines;
} scl;

void scale_surface(BYTE nidx) {
	scl.sx = 0;
	scl.sy = 0;
	scl.oy = 0;
	scl.lines = SCR_ROWS;
	scl.rows = SCR_COLUMNS;
	scl.startx = 0;

	scale_surface1x(nes[nidx].p.ppu_screen.rd, (uint32_t *)gfx.filter.data.palette,
		gfx.filter.data.pitch, gfx.filter.data.pix);
}
void scale_surface_screenshoot_1x(BYTE nidx, uint32_t pitch, void *pix) {
	scl.sx = 0;
	scl.sy = 0;
	scl.oy = 0;
	scl.lines = SCR_ROWS;
	scl.rows = SCR_COLUMNS;
	scl.startx = 0;

	scale_surface1x(nes[nidx].p.ppu_screen.rd, gfx.palette, pitch, pix);
}
void scale_surface_preview_1x(void *sb, uint32_t pitch, void *pix) {
	scl.sx = 0;
	scl.sy = 0;
	scl.oy = 0;
	scl.lines = SCR_ROWS;
	scl.rows = SCR_COLUMNS;
	scl.startx = 0;

	scale_surface1x((_ppu_screen_buffer *)sb, gfx.palette, pitch, pix);
}

INLINE void scale_surface1x(_ppu_screen_buffer *sb, const uint32_t *palette, uint32_t pitch, void *pix) {
	const uint32_t dstpitch = pitch;
	uint8_t *dstpix = (uint8_t *)pix;
	uint32_t TH0 = 0, TW0 = 0;
	uint32_t pixel = 0;

	for (; scl.sy < scl.lines; scl.sy++) {
		TH0 = (scl.oy * dstpitch);
		scl.ox = 0;
		/* loop per l'intera larghezza dell'immagine */
		for (scl.sx = scl.startx; scl.sx < scl.rows; scl.sx++) {
			pixel = palette[sb->line[scl.sy][scl.sx]];
			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			TW0 = (scl.ox << 2);
			put_pixel(uint32_t, TH0, TW0);
			scl.ox++;
		}
		scl.oy++;
	}
}
