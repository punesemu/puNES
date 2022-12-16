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

#include <stdlib.h>
#include <math.h>
#include "tv_noise.h"
#include "ppu.h"
#include "conf.h"
#include "video/gfx.h"
#include "emu.h"
#include "palette.h"
#include "gui.h"

_turn_off_effect turn_off_effect;

BYTE tv_noise_init(void) {
	uint32_t *palette;
	_color_RGB pRGB[NUM_COLORS];
	WORD i;

	if (!(turn_off_effect.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		log_error(uL("tv_noise;unable to allocate the palette"));
		return (EXIT_ERROR);
	}
	palette = (uint32_t *)turn_off_effect.palette;

	if (!(turn_off_effect.ntsc = malloc(sizeof(nes_ntsc_t)))) {
		log_error(uL("tv_noise;unable to allocate the palette"));
		return (EXIT_ERROR);
	}

	ntsc_rgb_modifier((nes_ntsc_t *)turn_off_effect.ntsc, (BYTE *)pRGB, 0x1A, -0x20, -0x30, -0x20);
	//ntsc_rgb_modifier((nes_ntsc_t *)turn_off_effect.ntsc, (BYTE *)pRGB, 0x00, -0x20, -0x20, -0x20);

	for (i = 0; i < NUM_COLORS; i++) {
		palette[i] = gfx_color(255, pRGB[i].r, pRGB[i].g, pRGB[i].b);
	}

	return (EXIT_OK);
}
void tv_noise_quit(void) {
	if (turn_off_effect.palette) {
		free(turn_off_effect.palette);
		turn_off_effect.palette = NULL;
	}
	if (turn_off_effect.ntsc) {
		free(turn_off_effect.ntsc);
		turn_off_effect.ntsc = NULL;
	}
}
void tv_noise_effect(void) {
	static WORD t0 = 0;
	BYTE direction = 1;
	int x, y;

	if (cfg->disable_tv_noise) {
		for (y = 0; y < SCR_ROWS; y++) {
			for (x = 0; x < SCR_COLUMNS; x++) {
				ppu_screen.wr->line[y][x] = 0x0D;
			}
		}
		return;
	}

	for (y = 0; y < SCR_ROWS; y++) {
		for (x = 0; x < SCR_COLUMNS; x++) {
			WORD w = (WORD)(7 + sin((double)x / 50000 + (double)t0 / 7));

			ppu_screen.wr->line[y][x] = emu_irand(16) * w;
		}
		t0 = (t0 + 1) % SCR_ROWS;
	}

	if (direction == 0) {
		t0 = (t0 + 1) % SCR_ROWS;
	} else {
		t0 = (t0 - 1) % SCR_ROWS;

		if (t0 == 0xFFFF) {
			t0 = SCR_ROWS - 1;
		}
	}
}
