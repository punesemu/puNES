/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include "tv_noise.h"
#include "ppu.h"
#include "conf.h"
#include "gfx.h"
#include "emu.h"
#include "video/filters/ntsc.h"

BYTE tv_noise_init(void) {
	_color_RGB pRGB[NUM_COLORS];
	WORD i;

	if (!(turn_off.palette = (uint32_t *) malloc(NUM_COLORS * sizeof(uint32_t)))) {
		fprintf(stderr, "Unable to allocate the palette\n");
		return (EXIT_ERROR);
	}

	rgb_modifier(pRGB, 0x1A, -0x20, -0x30, -0x20);
	//rgb_modifier(pRGB, 0x00, -0x20, -0x20, -0x20);

	for (i = 0; i < NUM_COLORS; i++) {
		turn_off.palette[i] = gfx_color(255, pRGB[i].r, pRGB[i].g, pRGB[i].b);
	}

	return (EXIT_OK);
}
void tv_noise_quit(void) {
	if (turn_off.palette) {
		free(turn_off.palette);
		turn_off.palette = NULL;
	}
}
void tv_noise_effect(void) {
	static WORD t0 = 0;
	BYTE direction = 1;
	WORD x, y;

	for (y = 0; y < SCR_LINES; y++) {
		for (x = 0; x < SCR_ROWS; x++) {
			WORD w = 7 + sin(x / 50000 + t0 / 7);
			screen.line[y][x] = emu_irand(16) * w;
		}
		t0 = (t0 + 1) % SCR_LINES;
	}

	if (direction == 0) {
		t0 = (t0 + 1) % SCR_LINES;
	} else {
		t0 = (t0 - 1) % SCR_LINES;

		if (t0 == 0xFFFF) {
			t0 = SCR_LINES - 1;
		}
	}
}

