/*
 * bilinear.c
 *
 *  Created on: 14/gen/2012
 *      Author: fhorse
 */

#include "bilinear.h"
#include "overscan.h"
#include "palette.h"

gfx_filter_function(bilinear) {
	int32_t *dstpix = (int32_t *) pix;
	WORD ox = 0, oy = 0;
	_color_RGB A, B, C, D;
	unsigned int w_step_fixed_16b, h_step_fixed_16b, w_coef, h_coef, x, y;
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

	w_step_fixed_16b = ((rows - 1) << 16) / (width - 1);
	h_step_fixed_16b = ((lines - 1) << 16) / (height - 1);

	h_coef = 0;

	for (y = 0; y < height; y++) {
		offsetY[0] = (h_coef >> 16) + oy;
		hc2 = (h_coef >> 9) & 127;
		hc1 = 128 - hc2;

		w_coef = 0;
		for (x = 0; x < width; x++) {
			uint8_t pippo = FALSE;
			uint8_t pluto = FALSE;
			uint8_t minni = FALSE;

			offsetX[0] = (w_coef >> 16) + ox;
			wc2[0] = (w_coef >> 9) & 127;
			wc1[0] = 128 - wc2[0];

			if (offsetX[0] != offsetX[1]) {
				A = palette_RGB[(screen_index[offsetY[0]][offsetX[0]])];
				B = palette_RGB[(screen_index[offsetY[0] + 1][offsetX[0]])];
				C = palette_RGB[(screen_index[offsetY[0]][offsetX[0] + 1])];
				D = palette_RGB[(screen_index[offsetY[0] + 1][offsetX[0] + 1])];

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

			w_coef += w_step_fixed_16b;
		}
		h_coef += h_step_fixed_16b;
	}

	return;

	/*
	w_step_fixed_16b = ((SCR_ROWS - 1) << 16) / (dst->w - 1);
	h_step_fixed_16b = ((SCR_LINES - 1) << 16) / (dst->h - 1);

	h_coef = 0;

	SDL_LockSurface(dst);

	for (y = 0; y < dst->h; y++) {
		offsetY = (h_coef >> 16);
		hc2 = (h_coef >> 9) & 127;
		hc1 = 128 - hc2;

		w_coef = 0;
		for (x = 0; x < dst->w; x++) {
			offsetX = (w_coef >> 16);
			wc2 = (w_coef >> 9) & 127;
			wc1 = 128 - wc2;

			A = palette_RGB[(screen_index[offsetY][offsetX])];
			B = palette_RGB[(screen_index[offsetY + 1][offsetX])];
			C = palette_RGB[(screen_index[offsetY][offsetX + 1])];
			D = palette_RGB[(screen_index[offsetY + 1][offsetX + 1])];

			r = ((A.r * hc1 + B.r * hc2) * wc1 + (C.r * hc1 + D.r * hc2) * wc2) >> 14;
			g = ((A.g * hc1 + B.g * hc2) * wc1 + (C.g * hc1 + D.g * hc2) * wc2) >> 14;
			b = ((A.b * hc1 + B.b * hc2) * wc1 + (C.b * hc1 + D.b * hc2) * wc2) >> 14;

			*dstpix++ = (r << 16) | (g << 8) | b;

			w_coef += w_step_fixed_16b;
		}
		h_coef += h_step_fixed_16b;
	}

	SDL_UnlockSurface(dst);

	return;

	*/
}
