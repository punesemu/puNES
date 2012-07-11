/*
 * hqx.c
 *
 *  Created on: 04/gen/2012
 *      Author: fhorse
 */

#include "hqx.h"
#include "overscan.h"

uint32_t RGBtoYUV[NCOLORS];
uint32_t YUV1, YUV2;

void hqxInit(void) {
	/* Initalize RGB to YUV lookup table */
	uint32_t i, r, g, b, y, u, v;

	for (i = 0; i < NCOLORS; i++) {
		r = paletteRGB[i].r;
		g = paletteRGB[i].g;
		b = paletteRGB[i].b;
		y = (uint32_t) (0.299 * r + 0.587 * g + 0.114 * b);
		u = (uint32_t) (-0.169 * r - 0.331 * g + 0.5 * b) + 128;
		v = (uint32_t) (0.5 * r - 0.419 * g - 0.081 * b) + 128;
		RGBtoYUV[i] = (y << 16) + (u << 8) + v;
	}

}
void hqNx(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor) {
	hqnx.sx = 0;
	hqnx.sy = 0;
	hqnx.lines = lines;
	hqnx.rows = hqnx.dst_rows = rows;
	hqnx.startx = 0;

	if (overscan.enabled) {
		hqnx.sy += overscan.up;
		hqnx.lines += overscan.up;
		hqnx.rows += overscan.left;
		hqnx.startx = overscan.left;
		screen += (SCRROWS * overscan.up);
	}

	if (factor == 1) {
		return;
	} else if (factor == 2) {
		hq2x_32_rb(screen, dst, palette);
	} else if (factor == 3) {
		hq3x_32_rb(screen, dst, palette);
	} else if (factor == 4) {
		hq4x_32_rb(screen, dst, palette);
	}
}
