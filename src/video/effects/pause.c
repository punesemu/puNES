/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include "pause.h"
#include "video/gfx.h"
#include "palette.h"
#include "gui.h"

_pause_effect pause_effect;

BYTE pause_init(void) {
	_color_RGB pRGB[NUM_COLORS] = { 0 };
	uint32_t *palette = NULL;

	pause_effect.frames = 0;

	if (!(pause_effect.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		log_error(uL("pause;unable to allocate the palette"));
		return (EXIT_ERROR);
	}
	palette = (uint32_t *)pause_effect.palette;

	if (!(pause_effect.ntsc = malloc(sizeof(nes_ntsc_t)))) {
		log_error(uL("pause;unable to allocate the palette"));
		return (EXIT_ERROR);
	}

	ntsc_rgb_modifier((nes_ntsc_t *)pause_effect.ntsc, (BYTE *)pRGB, 0x1A, -0x0A, -0x0A, -0x30);

	for (int i = 0; i < NUM_COLORS; i++) {
		palette[i] = gfx_color(255, pRGB[i].r, pRGB[i].g, pRGB[i].b);
	}

	return (EXIT_OK);
}
void pause_quit(void) {
	if (pause_effect.palette) {
		free(pause_effect.palette);
		pause_effect.palette = NULL;
	}
	if (pause_effect.ntsc) {
		free(pause_effect.ntsc);
		pause_effect.ntsc = NULL;
	}
}
