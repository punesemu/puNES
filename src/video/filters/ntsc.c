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

#if defined (D3D9)
#include <stdio.h>
#include <stdlib.h>
#endif
#include "video/filters/ntsc.h"
#include "overscan.h"

nes_ntsc_t *ntsc;
nes_ntsc_setup_t format[3];
int merge_fields = 1;
int burst_phase = 0;

#define adjust_output(scale)\
	for (y = ((height / factor) - (overscan.enabled ? 0 : 1)); --y >= 0;) {\
		unsigned char const *in = ((const unsigned char *) pix) + (y * pitch);\
		unsigned char *out = ((unsigned char *) pix) + ((y * factor) * pitch);\
		int n;\
		for (n = width; n; --n) {\
			switch (bpp) {\
				case 15:\
				case 16:\
					scale(uint16_t, 0x0821, 0x18E3, 0x0000);\
					break;\
				case 24:\
				case 32:\
					scale(uint32_t, 0x00010101, 0x00030703, 0xFF000000);\
					break;\
			}\
			in += bpp / 8;\
			out += bpp / 8;\
		}\
	}
#define DOUBLE(type, mask_low_bits, mask_darken, mask_alpha)\
{\
	unsigned prev = *(type *) in;\
	unsigned next = *(type *) (in + pitch);\
	/* mix rgb without losing low bits */\
	unsigned mixed = prev + next + ((prev ^ next) & mask_low_bits);\
	/* darken by 12% */\
	*(type *) out = prev | mask_alpha;\
	*(type *) (out + pitch) = ((mixed >> 1) - (mixed >> 4 & mask_darken)) | mask_alpha;\
}
#define TRIPLE(type, mask_low_bits, mask_darken, mask_alpha)\
{\
	unsigned prev = *(type *) in;\
	unsigned next = *(type *) (in + pitch);\
	/* mix rgb without losing low bits */\
	unsigned mixed = prev + next + ((prev ^ next) & mask_low_bits);\
	/* darken by 12% */\
	*(type *) out = prev | mask_alpha;\
	*(type *) (out + pitch) = ((mixed >> 1) - (mixed >> 2 & mask_darken)) | mask_alpha ;\
	*(type *) (out + pitch + pitch) = ((mixed >> 1) - (mixed >> 4 & mask_darken)) | mask_alpha;\
}
#define QUADRUPLE(type, mask_low_bits, mask_darken, mask_alpha)\
{\
	unsigned prev = *(type *) in;\
	unsigned next = *(type *) (in + pitch);\
	/* mix rgb without losing low bits */\
	unsigned mixed = prev + next + ((prev ^ next) & mask_low_bits);\
	/* darken by 12% */\
	*(type *) out = *(type *) (out + pitch) = prev | mask_alpha;\
	*(type *) (out + (pitch << 1)) = *(type *) (out + ((pitch << 1) + pitch)) =\
			((mixed >> 1) - (mixed >> 4 & mask_darken)) | mask_alpha;\
}
#define nes_ntsc(factor) nes_ntscx##factor(ntsc, screen, SCR_ROWS, burst_phase, rows, lines,\
	pix, pitch, bpp)

/*
 * cio' che non utilizzo in questa funzione
 * sono i parametri WORD **screen_index e
 * Uint32 *palette.
 */
gfx_filter_function(ntsc_surface) {
	int y;

	if (overscan.enabled) {
		screen += (SCR_ROWS * overscan.borders->up);
	}

	if (factor == 1) {
		return;
	} else if (factor == 2) {
		nes_ntsc(2);
		adjust_output(DOUBLE)
	} else if (factor == 3) {
		nes_ntsc(3);
		adjust_output(TRIPLE);
	} else if (factor == 4) {
		nes_ntsc(4);
		adjust_output(QUADRUPLE)
	}
}

BYTE ntsc_init(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out) {
	format[COMPOSITE] = nes_ntsc_composite;
	format[SVIDEO] = nes_ntsc_svideo;
	format[RGBMODE] = nes_ntsc_rgb;

	ntsc = (nes_ntsc_t *) malloc(sizeof(nes_ntsc_t));
	if (!ntsc) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	ntsc_set(effect, color, palette_base, palette_in, palette_out);
	return (EXIT_OK);
}
void ntsc_quit(void) {
	free(ntsc);
}
void ntsc_set(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out) {
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

	nes_ntsc_init(ntsc, &format[effect]);
}
