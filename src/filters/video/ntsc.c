/*
 * ntsc.c
 *
 *  Created on: 03/ago/2010
 *      Author: fhorse
 */

#include "ntsc.h"
#include "overscan.h"

nes_ntsc_t *ntsc;
nes_ntsc_setup_t format[3];
int merge_fields = 1;
int burst_phase = 0;

#define ADJUSTOUTPUT(scale)\
		for (y = ((dst->h / factor) - (overscan.enabled ? 0 : 1)); --y >= 0;) {\
			unsigned char const *in = dst->pixels + (y * dst->pitch);\
			unsigned char *out = dst->pixels + ((y * factor) * dst->pitch);\
			int n;\
			for (n = dst->w; n; --n) {\
				switch (dst->format->BitsPerPixel) {\
					case 15:\
					case 16:\
						scale(Uint16, 0x0821, 0x18E3);\
						break;\
					case 24:\
					case 32:\
						scale(Uint32, 0x00010101, 0x00030703);\
						break;\
				}\
				in += dst->format->BytesPerPixel;\
				out += dst->format->BytesPerPixel;\
			}\
		}
#define DOUBLE(type, maskLowBits, maskDarken)\
	{\
		unsigned prev = *(type *) in;\
		unsigned next = *(type *) (in + dst->pitch);\
		/* mix rgb without losing low bits */\
		unsigned mixed = prev + next + ((prev ^ next) & maskLowBits);\
		/* darken by 12% */\
		*(type *) out = prev;\
		*(type *) (out + dst->pitch) = (mixed >> 1) - (mixed >> 4\
				& maskDarken);\
	}
#define TRIPLE(type, maskLowBits, maskDarken)\
	{\
		unsigned prev = *(type *) in;\
		unsigned next = *(type *) (in + dst->pitch);\
		/* mix rgb without losing low bits */\
		unsigned mixed = prev + next + ((prev ^ next) & maskLowBits);\
		/* darken by 12% */\
		*(type *) out = prev;\
		*(type *) (out + dst->pitch) = (mixed >> 1) - (mixed >> 2\
				& maskDarken);\
		*(type *) (out + dst->pitch + dst->pitch) = (mixed >> 1) - (mixed >> 4\
				& maskDarken);\
	}
#define QUADRUPLE(type, maskLowBits, maskDarken)\
	{\
		unsigned prev = *(type *) in;\
		unsigned next = *(type *) (in + dst->pitch);\
		/* mix rgb without losing low bits */\
		unsigned mixed = prev + next + ((prev ^ next) & maskLowBits);\
		/* darken by 12% */\
		*(type *) out = *(type *) (out + dst->pitch) = prev;\
		*(type *) (out + (dst->pitch << 1)) =\
			*(type *) (out + ((dst->pitch << 1) + dst->pitch)) =\
				(mixed >> 1) - (mixed >> 4 & maskDarken);\
	}
#define NESNTSC(factor) nes_ntscx##factor(ntsc,\
		screen, SCRROWS, burst_phase, rows,\
		lines, dst->pixels, dst->pitch, dst->format->BitsPerPixel)

/*
 * cio' che non utilizzo in questa funzione
 * sono i parametri WORD **screen_index e
 * Uint32 *palette.
 */
void ntscSurface(WORD *screen, WORD **screeIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor) {
	int y;

	if (overscan.enabled) {
		screen += (SCRROWS * overscan.up);
	}

	/* lock della destinazione */
	//SDL_LockSurface(dst);
	if (factor == 1) {
		return;
	} else if (factor == 2) {
		NESNTSC(2);
		ADJUSTOUTPUT(DOUBLE)
	} else if (factor == 3) {
		NESNTSC(3);
		ADJUSTOUTPUT(TRIPLE);
	} else if (factor == 4) {
		NESNTSC(4);
		ADJUSTOUTPUT(QUADRUPLE)
	}
	/* unlock della destinazione */
	//SDL_UnlockSurface(dst);

	/*
	 switch ( key_pressed )
	{
		case ' ': merge_fields = !merge_fields; break;
		case 'c': setup = nes_ntsc_composite; break;
		case 's': setup = nes_ntsc_svideo; break;
		case 'r': setup = nes_ntsc_rgb; break;
		case 'm': setup = nes_ntsc_monochrome; break;
		case 'd': sony_decoder = !sony_decoder; break;
	}
*/

/*
	if ( key_pressed || mouse_moved )
	{
		setup.merge_fields = merge_fields;

*/
}
BYTE ntscInit(BYTE effect, BYTE color, BYTE *paletteBase, BYTE *paletteIN, BYTE *paletteOUT) {
	format[COMPOSITE] = nes_ntsc_composite;
	format[SVIDEO] = nes_ntsc_svideo;
	format[RGBMODE] = nes_ntsc_rgb;

	ntsc = (nes_ntsc_t *) malloc(sizeof(nes_ntsc_t));
	if (!ntsc) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}
	ntscSet(effect, color, paletteBase, paletteIN, paletteOUT);
	return (EXIT_OK);
}
void ntscQuit(void) {
	free(ntsc);
}
void ntscSet(BYTE effect, BYTE color, BYTE *paletteBase, BYTE *paletteIN, BYTE *paletteOUT) {
	if (paletteBase) {
		format[effect].base_palette = (unsigned char *) paletteBase;
	} else {
		format[effect].base_palette = 0;
	}

	if (paletteIN) {
		format[effect].palette = (unsigned char *) paletteIN;
	} else {
		format[effect].palette = 0;
	}

	if (paletteOUT) {
		format[effect].palette_out = (unsigned char *) paletteOUT;
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
