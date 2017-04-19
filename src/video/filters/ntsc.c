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

#if defined (WITH_D3D9)
#include <stdio.h>
#include <stdlib.h>
#endif
#include "video/filters/ntsc.h"
#include "overscan.h"

nes_ntsc_t *ntsc;
nes_ntsc_setup_t format[3];
int merge_fields = 1;
int burst_phase = 0;

/*
 * cio' che non utilizzo in questa funzione
 * sono i parametri WORD **screen_index e
 * Uint32 *palette.
 */
gfx_filter_function(ntsc_surface) {
	int y;

	if (palette == NULL) {
		palette = (void *) ntsc;
	}

	nes_ntsc_blit((nes_ntsc_t *) palette, screen, SCR_ROWS, burst_phase, SCR_ROWS, SCR_LINES, pix,
			pitch);

	for (y = ((height / gfx.filter.factor) - 1); --y >= 0;) {
		unsigned char const *in = ((const unsigned char *) pix) + (y * pitch);
		unsigned char *out = ((unsigned char *) pix) + ((y * gfx.filter.factor) * pitch);
		int n;

		for (n = width; n; --n) {
			unsigned prev = *(uint32_t *) in;
			unsigned next = *(uint32_t *) (in + pitch);
			/* mix rgb without losing low bits */
			unsigned mixed = prev + next + ((prev ^ next) & 0x00010101);

			/* darken by 12% */
			*(uint32_t *) out = prev | 0xFF000000;
			*(uint32_t *) (out + pitch) = ((mixed >> 1) - (mixed >> 4 & 0x00030703)) | 0xFF000000;

			in += NES_NTSC_OUT_DEPTH / 8;
			out += NES_NTSC_OUT_DEPTH / 8;
		}
	}
}

BYTE ntsc_init(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out) {
	format[COMPOSITE] = nes_ntsc_composite;
	format[SVIDEO] = nes_ntsc_svideo;
	format[RGBMODE] = nes_ntsc_rgb;

	if (!(ntsc = (nes_ntsc_t *) malloc(sizeof(nes_ntsc_t)))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	ntsc_set(NULL, effect, color, palette_base, palette_in, palette_out);
	return (EXIT_OK);
}
void ntsc_quit(void) {
	free(ntsc);
}
void ntsc_set(nes_ntsc_t *ntsc_in, BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in,
		BYTE *palette_out) {
	if (!ntsc_in) {
		ntsc_in = ntsc;
	}

	if (palette_base) {
		format[effect].base_palette = (unsigned char *) palette_base;
	} else {
		format[effect].base_palette = 0;
	}

	if (palette_in) {
		format[effect].palette = (unsigned char *) palette_in;
	} else {
		format[effect].palette = 0;
	}

	if (palette_out) {
		format[effect].palette_out = (unsigned char *) palette_out;
	} else {
		format[effect].palette_out = 0;
	}

	//burst_phase ^= 1;
	/*if ( setup.merge_fields ) {
	 burst_phase = 0;
	 }*/
	//format[effect].merge_fields = merge_fields;
	format[effect].decoder_matrix = 0;
	format[effect].saturation = 0;

	if (color) {
		switch (color) {
			/* Sony CXA2025AS US */
			case PALETTE_SONY: {
				static float matrix[6] = { 1.630, 0.317, -0.378, -0.466, -1.089, 1.677 };

				format[effect].decoder_matrix = matrix;
				break;
			}
			case PALETTE_MONO:
				format[effect].saturation = -1;
				break;
		}
	}

	nes_ntsc_init(ntsc_in, &format[effect]);
}
