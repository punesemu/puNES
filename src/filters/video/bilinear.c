/*
 * bilinear.c
 *
 *  Created on: 14/gen/2012
 *      Author: fhorse
 */

#include "bilinear.h"
#include "overscan.h"
#include "palette.h"

void bilinear(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
        WORD lines, BYTE factor) {
	int32_t *dstpix = (int32_t *) dst->pixels;
	WORD ox = 0, oy = 0;
	_colorRGB A, B, C, D;
	unsigned int wStepFixed16b, hStepFixed16b, wCoef, hCoef, x, y;
	unsigned int hc1, hc2;
	unsigned int wc1[2] = {0, 0};
	unsigned int wc2[2] = {0, 0};
	unsigned int offsetX[2] = {0, 0};
	unsigned int offsetY[2] = {0, 0};
	unsigned int r, g, b;
	unsigned int ra[2] = {0, 0};
	unsigned int ga[2] = {0, 0};
	unsigned int ba[2] = {0, 0};
	unsigned int rb[2] = {0, 0};
	unsigned int gb[2] = {0, 0};
	unsigned int bb[2] = {0, 0};
	uint32_t ziopapero = 0;

	if (overscan.enabled) {
		ox = overscan.left;
		oy = overscan.up;
	}

	/* lock della destinazione */
	//SDL_LockSurface(dst);

	wStepFixed16b = ((rows - 1) << 16) / (dst->w - 1);
	hStepFixed16b = ((lines - 1) << 16) / (dst->h - 1);

	hCoef = 0;

	for (y = 0; y < dst->h; y++) {
		offsetY[0] = (hCoef >> 16) + oy;
		hc2 = (hCoef >> 9) & 127;
		hc1 = 128 - hc2;

		wCoef = 0;
		for (x = 0; x < dst->w; x++) {
			uint8_t pippo = FALSE;
			uint8_t pluto = FALSE;
			uint8_t minni = FALSE;

			offsetX[0] = (wCoef >> 16) + ox;
			wc2[0] = (wCoef >> 9) & 127;
			wc1[0] = 128 - wc2[0];

			if (offsetX[0] != offsetX[1]) {
				A = paletteRGB[(screenIndex[offsetY[0]][offsetX[0]])];
				B = paletteRGB[(screenIndex[offsetY[0] + 1][offsetX[0]])];
				C = paletteRGB[(screenIndex[offsetY[0]][offsetX[0] + 1])];
				D = paletteRGB[(screenIndex[offsetY[0] + 1][offsetX[0] + 1])];

				ra[0] = (A.r * hc1 + B.r * hc2);
				ra[1] = (C.r * hc1 + D.r * hc2);
				ga[0] = (A.g * hc1 + B.g * hc2);
				ga[1] = (C.g * hc1 + D.g * hc2);
				ba[0] = (A.b * hc1 + B.b * hc2);
				ba[1] = (C.b * hc1 + D.b * hc2);

				offsetX[1] = offsetX[0];
				pippo = TRUE;
			}

			if ((wc1[0] != wc1[1]) || pippo) {
				rb[0] = (ra[0] * wc1[0]);
				gb[0] = (ga[0] * wc1[0]);
				bb[0] = (ba[0] * wc1[0]);

				wc1[1] = wc1[0];
				pluto = TRUE;
			}

			if ((wc2[0] != wc2[1]) || pippo) {
				rb[1] = (ra[1] * wc2[0]);
				gb[1] = (ga[1] * wc2[0]);
				bb[1] = (ba[1] * wc2[0]);

				wc2[1] = wc2[0];
				minni = TRUE;
			}

			if (pippo || pluto || minni) {
				r = (rb[0] + rb[1]) >> 14;
				g = (gb[0] + gb[1]) >> 14;
				b = (bb[0] + bb[1]) >> 14;
				ziopapero = (r << 16) | (g << 8) | b;
			}

			(*dstpix++) = ziopapero;

			wCoef += wStepFixed16b;
		}
		hCoef += hStepFixed16b;
	}

	/* unlock della destinazione */
	//SDL_UnlockSurface(dst);

	return;

	/*
	wStepFixed16b = ((SCRROWS - 1) << 16) / (dst->w - 1);
	hStepFixed16b = ((SCRLINES - 1) << 16) / (dst->h - 1);

	hCoef = 0;

	SDL_LockSurface(dst);

	for (y = 0; y < dst->h; y++) {
		offsetY = (hCoef >> 16);
		hc2 = (hCoef >> 9) & 127;
		hc1 = 128 - hc2;

		wCoef = 0;
		for (x = 0; x < dst->w; x++) {
			offsetX = (wCoef >> 16);
			wc2 = (wCoef >> 9) & 127;
			wc1 = 128 - wc2;

			A = paletteRGB[(screenIndex[offsetY][offsetX])];
			B = paletteRGB[(screenIndex[offsetY + 1][offsetX])];
			C = paletteRGB[(screenIndex[offsetY][offsetX + 1])];
			D = paletteRGB[(screenIndex[offsetY + 1][offsetX + 1])];

			r = ((A.r * hc1 + B.r * hc2) * wc1 + (C.r * hc1 + D.r * hc2) * wc2) >> 14;
			g = ((A.g * hc1 + B.g * hc2) * wc1 + (C.g * hc1 + D.g * hc2) * wc2) >> 14;
			b = ((A.b * hc1 + B.b * hc2) * wc1 + (C.b * hc1 + D.b * hc2) * wc2) >> 14;

			*dstpix++ = (r << 16) | (g << 8) | b;

			wCoef += wStepFixed16b;
		}
		hCoef += hStepFixed16b;
	}

	SDL_UnlockSurface(dst);

	return;

	*/
}
