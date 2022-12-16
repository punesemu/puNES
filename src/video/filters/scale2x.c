/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <stdlib.h>
#include "video/gfx.h"
#include "ppu.h"
#include "gui.h"

#define READINT24(x) ((x)[0] << 16 | (x)[1] << 8 | (x)[2])
#define WRITEINT24(x, i)\
	x[0] = i >> 16;\
	x[1] = (i >> 8) & 0xff;\
	x[2] = i & 0xff
#define X3(a) ((a << 1) + a)
#define SCALE2X()\
	if (B != H && D != F) {\
		E0 = D == B ? D : E;\
		E1 = B == F ? F : E;\
		E2 = D == H ? D : E;\
		E3 = H == F ? F : E;\
	} else {\
		E0 = E1 = E2 = E3 = E;\
	}
#define SCALE3X_A()\
	E0 = D == B ? D : E;\
	E1 = (D == B && E != C) || (B == F && E != A) ? B : E;\
	E2 = B == F ? F : E;\
	E3 = (D == B && E != G) || (D == H && E != A) ? D : E;\
	E4 = E;\
	E5 = (B == F && E != I) || (H == F && E != C) ? F : E;\
	E6 = D == H ? D : E;\
	E7 = (D == H && E != I) || (H == F && E != G) ? H : E;\
	E8 = H == F ? F : E;
#define SCALE3X_B()\
	E0 = E1 = E2 = E3 = E4 = E5 = E6 = E7 = E8 = E;
#define put_pixel(type, pixel, p0, p1)\
	*(type *)(dstpix + p0 + p1) = (type)palette[pixel]

INLINE static void scale2x(WORD **screen_index, const uint32_t *palette, uint32_t pitch, void *pix);
INLINE static void scale3x(WORD **screen_index, const uint32_t *palette, uint32_t pitch, void *pix);
INLINE static void scale4x(WORD **screen_index, const uint32_t *palette, uint32_t pitch, void *pix);

struct _scl2x {
	WORD sx;
	WORD sy;
	WORD oy;
	WORD ox;
	WORD startx;
	WORD rows;
	WORD lines;
} scl2x;
struct _s4x_buffer {
	uint32_t w, h;
	uint32_t pitch;
	uint32_t size;
	void *pixels;
} scl4x_buffer;

/*
 * cio' che non utilizzo in questa funzione
 * e' il parametro WORD *screen.
 */
void scaleNx(void) {
	scl2x.sx = 0;
	scl2x.sy = 0;
	scl2x.oy = 0;
	scl2x.lines = SCR_ROWS;
	scl2x.rows = SCR_COLUMNS;
	scl2x.startx = 0;

	if (gfx.filter.factor == 2) {
		scale2x(ppu_screen.rd->line, (uint32_t *)gfx.filter.data.palette, gfx.filter.data.pitch, gfx.filter.data.pix);
	} else if (gfx.filter.factor == 3) {
		scale3x(ppu_screen.rd->line, (uint32_t *)gfx.filter.data.palette, gfx.filter.data.pitch, gfx.filter.data.pix);
	} else if (gfx.filter.factor == 4) {
		scl4x_buffer.w = SCR_COLUMNS * 2;
		scl4x_buffer.h = SCR_ROWS * 2;
		scl4x_buffer.pitch = scl4x_buffer.w * sizeof(uint32_t);
		scl4x_buffer.size = scl4x_buffer.pitch * scl4x_buffer.h;
		scale4x(ppu_screen.rd->line, (uint32_t *)gfx.filter.data.palette, gfx.filter.data.pitch, gfx.filter.data.pix);
	}
}

INLINE static void scale2x(WORD **screen_index, const uint32_t *palette, uint32_t pitch, void *pix) {
	const DBWORD dstpitch = pitch;
	WORD E0, E1, E2, E3, B, D, E, F, H;
	uint8_t *dstpix = (uint8_t *)pix;
	uint32_t TH0, TH1, TH2, TH3, TH4;
	uint32_t TW0, TW1, TW2;

	for (; scl2x.sy < scl2x.lines; ++scl2x.sy) {
		TH0 = FHMAX(0, scl2x.sy - 1);
		TH1 = scl2x.sy;
		TH2 = FHMIN(scl2x.lines - 1, scl2x.sy + 1);
		TH3 = ((scl2x.oy << 1) * dstpitch);
		TH4 = TH3 + dstpitch;

		scl2x.ox = 0;

		for (scl2x.sx = scl2x.startx; scl2x.sx < scl2x.rows; ++scl2x.sx) {
			TW0 = FHMAX(0, scl2x.sx - 1);
			TW1 = scl2x.sx;
			TW2 = FHMIN(scl2x.rows - 1, scl2x.sx + 1);
			B = screen_index[TH0][TW1];
			D = screen_index[TH1][TW0];
			E = screen_index[TH1][TW1];
			F = screen_index[TH1][TW2];
			H = screen_index[TH2][TW1];
			SCALE2X()
			TW0 = (scl2x.ox << 3);
			TW1 = TW0 + 4;
			put_pixel(uint32_t, E0, TH3, TW0);
			put_pixel(uint32_t, E1, TH3, TW1);
			put_pixel(uint32_t, E2, TH4, TW0);
			put_pixel(uint32_t, E3, TH4, TW1);
			scl2x.ox++;
		}
		scl2x.oy++;
	}
}
INLINE static void scale3x(WORD **screen_index, const uint32_t *palette, uint32_t pitch, void *pix) {
	const DBWORD dstpitch = pitch;
	WORD A, B, C, D, E, F, G, H, I;
	WORD E0, E1, E2, E3, E4, E5, E6, E7, E8;
	uint8_t *dstpix = (uint8_t *)pix;
	uint32_t TH0, TH1, TH2, TH3, TH4, TH5;
	uint32_t TW0, TW1, TW2;

	for (; scl2x.sy < scl2x.lines; ++scl2x.sy) {
		TH0 = FHMAX(0, scl2x.sy - 1);
		TH1 = scl2x.sy;
		TH2 = FHMIN(scl2x.lines - 1, scl2x.sy + 1);
		TH3 = (X3(scl2x.oy) * dstpitch);
		TH4 = TH3 + dstpitch;
		TH5 = TH3 + (dstpitch << 1);

		scl2x.ox = 0;

		for (scl2x.sx = scl2x.startx; scl2x.sx < scl2x.rows; ++scl2x.sx) {
			TW0 = FHMAX(0, scl2x.sx - 1);
			TW1 = scl2x.sx;
			TW2 = FHMIN(scl2x.rows - 1, scl2x.sx + 1);
			B = screen_index[TH0][TW1];
			D = screen_index[TH1][TW0];
			E = screen_index[TH1][TW1];
			F = screen_index[TH1][TW2];
			H = screen_index[TH2][TW1];
			if (B != H && D != F) {
				A = screen_index[TH0][TW0];
				C = screen_index[TH0][TW2];
				G = screen_index[TH2][TW0];
				I = screen_index[TH2][TW2];
				SCALE3X_A()
			} else {
				SCALE3X_B()
			}
			TW0 = (X3(scl2x.ox) << 2);
			TW1 = TW0 + 4;
			TW2 = TW0 + 8;
			put_pixel(uint32_t, E0, TH3, TW0);
			put_pixel(uint32_t, E1, TH3, TW1);
			put_pixel(uint32_t, E2, TH3, TW2);
			put_pixel(uint32_t, E3, TH4, TW0);
			put_pixel(uint32_t, E4, TH4, TW1);
			put_pixel(uint32_t, E5, TH4, TW2);
			put_pixel(uint32_t, E6, TH5, TW0);
			put_pixel(uint32_t, E7, TH5, TW1);
			put_pixel(uint32_t, E8, TH5, TW2);
			scl2x.ox++;
		}
		scl2x.oy++;
	}
}
INLINE static void scale4x(WORD **screen_index, const uint32_t *palette, uint32_t pitch, void *pix) {
	WORD x, y, width, height;
	DBWORD srcpitch, dstpitch = pitch;
	uint8_t *srcpix, *dstpix = (uint8_t *)pix;
	uint32_t TH0, TH1, TH2, TH3, TH4;
	uint32_t TW0, TW1, TW2;

	if ((scl4x_buffer.pixels = malloc(scl4x_buffer.size)) == NULL) {
		log_error(uL("scale4x;out of memory"));
		return;
	} else {
		scale2x(screen_index, palette, scl4x_buffer.pitch, scl4x_buffer.pixels);
	}

	srcpix = (uint8_t *)scl4x_buffer.pixels;
	srcpitch = scl4x_buffer.pitch;
	width = scl4x_buffer.w;
	height = scl4x_buffer.h;

	for (y = 0; y < height; ++y) {
		TH0 = (FHMAX(0, y - 1) * srcpitch);
		TH1 = (y * srcpitch);
		TH2 = (FHMIN(height - 1, y + 1) * srcpitch);
		TH3 = ((y << 1) * dstpitch);
		TH4 = TH3 + dstpitch;
		for (x = 0; x < width; ++x) {
			uint32_t E0, E1, E2, E3, B, D, E, F, H;

			TW0 = (FHMAX(0, x - 1) << 2);
			TW1 = (x << 2);
			TW2 = (FHMIN(width - 1, x + 1) << 2);
			B = *(uint32_t *)(srcpix + TH0 + TW1);
			D = *(uint32_t *)(srcpix + TH1 + TW0);
			E = *(uint32_t *)(srcpix + TH1 + TW1);
			F = *(uint32_t *)(srcpix + TH1 + TW2);
			H = *(uint32_t *)(srcpix + TH2 + TW1);
			SCALE2X()
			TW0 = (x << 3);
			TW1 = TW0 + 4;
			*(uint32_t *)(dstpix + TH3 + TW0) = E0;
			*(uint32_t *)(dstpix + TH3 + TW1) = E1;
			*(uint32_t *)(dstpix + TH4 + TW0) = E2;
			*(uint32_t *)(dstpix + TH4 + TW1) = E3;
		}
	}

	free(scl4x_buffer.pixels);
}
