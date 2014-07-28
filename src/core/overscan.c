/*
 * overscan.c
 *
 *  Created on: 23/mar/2014
 *      Author: fhorse
 */

#include "common.h"
#include "overscan.h"
#include "gfx.h"

BYTE overscan_set_mode(BYTE mode) {
	_overscan_borders save = (*overscan.borders);

	if (mode == NTSC) {
		overscan.borders = &overscan_borders[0];
	} else {
		overscan.borders = &overscan_borders[1];
	}

	{
		BYTE i, *src = (BYTE *) &save, *dst = (BYTE *) overscan.borders;

		for (i = 0; i < sizeof(_overscan_borders); i++) {
			if ((*(src + i)) != (*(dst + i))) {
				return (TRUE);
			}
		}
	}

	return (FALSE);
}
