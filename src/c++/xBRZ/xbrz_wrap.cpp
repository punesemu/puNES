/*
 * xbrz_wrap.c
 *
 *  Created on: 16/lug/2014
 *      Author: fhorse
 */

#include <c++/xBRZ/xbrz.h>

extern "C" void xbrz_scale(BYTE factor, const WORD *src, uint32_t *trg, uint32_t *palette,
		int no_overscan_width, int startx, int width, int height) {
	xbrz::scale(factor, src, trg, palette, no_overscan_width, startx, width, height);
}
