/*
 * xBRZ.c
 *
 *  Created on: 16/lug/2014
 *      Author: fhorse
 */

#include "video/filters/xBRZ.h"
#include "overscan.h"

void xBRZ_init(void) {
	;
}
gfx_filter_function(xBRZ) {
	if (overscan.enabled) {
		screen += (SCR_ROWS * overscan.borders->up);
	}

	if (factor == 1) {
		return;
	} else {
		xbrz_scale(factor, screen, (uint32_t *) pix, palette, SCR_ROWS,
		        (overscan.enabled == TRUE ? overscan.borders->left : 0), rows, lines);
	}
}
