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

#include <stdlib.h>
#include <math.h>
#include "pause.h"
#include "conf.h"
#include "gfx.h"
#include "video/filters/ntsc.h"

BYTE pause_init(void) {
	uint32_t *palette;
	_color_RGB pRGB[NUM_COLORS];
	WORD i;

	if (!(pause.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		fprintf(stderr, "Unable to allocate the palette\n");
		return (EXIT_ERROR);
	}
	palette = (uint32_t *) pause.palette;

	if (!(pause.ntsc = malloc(sizeof(nes_ntsc_t)))) {;
		fprintf(stderr, "Unable to allocate the palette\n");
		return (EXIT_ERROR);
	}

	rgb_modifier((nes_ntsc_t *) pause.ntsc, pRGB, 0x1A, -0x0A, -0x0A, -0x30);

	for (i = 0; i < NUM_COLORS; i++) {
		palette[i] = gfx_color(255, pRGB[i].r, pRGB[i].g, pRGB[i].b);
	}

	return (EXIT_OK);
}
void pause_quit(void) {
	if (pause.palette) {
		free(pause.palette);
		pause.palette = NULL;
	}
	if (pause.ntsc) {
		free(pause.ntsc);
		pause.ntsc = NULL;
	}
}
