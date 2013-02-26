/*
 * scale.c
 *
 *  Created on: 22/mag/2010
 *      Author: fhorse
 */

#include "scale.h"
#include "overscan.h"

#define X3(a) ((a << 1) + a)
#define put_pixel(type, p0, p1)\
    *(type *) (dstpix + p0 + p1) = (type) pixel

struct _scl {
	WORD sx;
	WORD sy;
	WORD oy;
	WORD ox;
	WORD startx;
	WORD rows;
	WORD lines;
} scl;

void scale_surface(WORD *screen, WORD **screen_index, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor) {

	scl.sx = 0;
	scl.sy = 0;
	scl.oy = 0;
	scl.lines = lines;
	scl.rows = rows;
	scl.startx = 0;

	if (overscan.enabled) {
		scl.sy += overscan.up;
		scl.lines += overscan.up;
		scl.rows += overscan.left;
		scl.startx = overscan.left;
	}

	if (factor == 1) {
		scale_surface1x(screen_index, palette, dst);
	} else if (factor == 2) {
		scale_surface2x(screen_index, palette, dst);
	} else if (factor == 3) {
		scale_surface3x(screen_index, palette, dst);
	} else if (factor == 4) {
		scale_surface4x(screen_index, palette, dst);
	}
}
void scale_surface1x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst) {
	const uint32_t dstpitch = dst->pitch;
	uint8_t *dstpix = (uint8_t *) dst->pixels;
	uint32_t TH0, TW0;
	uint32_t pixel;

	for (; scl.sy < scl.lines; scl.sy++) {
		TH0 = (scl.oy * dstpitch);
		scl.ox = 0;
		/* loop per l'intera larghezza dell'immagine */
		for (scl.sx = scl.startx; scl.sx < scl.rows; scl.sx++) {
			pixel = palette[screen_index[scl.sy][scl.sx]];
			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = (scl.ox << 1);
					put_pixel(uint16_t, TH0, TW0);
					break;
				case 24:
					TW0 = (scl.ox << 1) + 1;
					put_pixel(int, TH0, TW0);
					break;
				case 32:
					TW0 = (scl.ox << 2);
					put_pixel(uint32_t, TH0, TW0);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
void scale_surface2x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst) {
	const uint32_t dstpitch = dst->pitch;
	uint8_t *dstpix = (uint8_t *) dst->pixels;
	uint32_t TH0, TH1, TW0, TW1;
	uint32_t pixel;

	for (; scl.sy < scl.lines; scl.sy++) {
		TH0 = ((scl.oy << 1) * dstpitch);
		TH1 = TH0 + dstpitch;
		scl.ox = 0;
		/* loop per l'intera larghezza dell'immagine */
		for (scl.sx = scl.startx; scl.sx < scl.rows; scl.sx++) {
			pixel = palette[screen_index[scl.sy][scl.sx]];

			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = (scl.ox << 2);
					TW1 = TW0 + 2;
					put_pixel(uint16_t, TH0, TW0);
					put_pixel(uint16_t, TH0, TW1);
					put_pixel(uint16_t, TH1, TW0);
					put_pixel(uint16_t, TH1, TW1);
					break;
				case 24:
					TW0 = X3((scl.ox << 1));
					TW1 = TW0 + 3;
					put_pixel(int, TH0, TW0);
					put_pixel(int, TH0, TW1);
					put_pixel(int, TH1, TW0);
					put_pixel(int, TH1, TW1);
					break;
				case 32:
					TW0 = (scl.ox << 3);
					TW1 = TW0 + 4;
					put_pixel(uint32_t, TH0, TW0);
					put_pixel(uint32_t, TH0, TW1);
					put_pixel(uint32_t, TH1, TW0);
					put_pixel(uint32_t, TH1, TW1);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
void scale_surface3x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst) {
	const uint32_t dstpitch = dst->pitch;
	uint8_t *dstpix = (uint8_t *) dst->pixels;
	uint32_t TH0, TH1, TH2, TW0, TW1, TW2;
	uint32_t pixel;

	for (; scl.sy < scl.lines; scl.sy++) {
		TH0 = (X3(scl.oy) * dstpitch);
		TH1 = TH0 + dstpitch;
		TH2 = TH1 + dstpitch;

		scl.ox = 0;
		/* loop per l'intera larghezza dell'immagine */
		for (scl.sx = scl.startx; scl.sx < scl.rows; scl.sx++) {
			pixel = palette[screen_index[scl.sy][scl.sx]];

			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = (X3(scl.ox) << 1);
					TW1 = TW0 + 2;
					TW2 = TW0 + 4;
					put_pixel(uint16_t, TH0, TW0);
					put_pixel(uint16_t, TH0, TW1);
					put_pixel(uint16_t, TH0, TW2);
					put_pixel(uint16_t, TH1, TW0);
					put_pixel(uint16_t, TH1, TW1);
					put_pixel(uint16_t, TH1, TW2);
					put_pixel(uint16_t, TH2, TW0);
					put_pixel(uint16_t, TH2, TW1);
					put_pixel(uint16_t, TH2, TW2);
					break;
				case 24:
					TW0 = X3((X3(scl.ox)));
					TW1 = TW0 + 3;
					TW2 = TW0 + 6;
					put_pixel(int, TH0, TW0);
					put_pixel(int, TH0, TW1);
					put_pixel(int, TH0, TW2);
					put_pixel(int, TH1, TW0);
					put_pixel(int, TH1, TW1);
					put_pixel(int, TH1, TW2);
					put_pixel(int, TH2, TW0);
					put_pixel(int, TH2, TW1);
					put_pixel(int, TH2, TW2);
					break;
				case 32:
					TW0 = (X3(scl.ox) << 2);
					TW1 = TW0 + 4;
					TW2 = TW0 + 8;
					put_pixel(uint32_t, TH0, TW0);
					put_pixel(uint32_t, TH0, TW1);
					put_pixel(uint32_t, TH0, TW2);
					put_pixel(uint32_t, TH1, TW0);
					put_pixel(uint32_t, TH1, TW1);
					put_pixel(uint32_t, TH1, TW2);
					put_pixel(uint32_t, TH2, TW0);
					put_pixel(uint32_t, TH2, TW1);
					put_pixel(uint32_t, TH2, TW2);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
void scale_surface4x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst) {
	const uint32_t dstpitch = dst->pitch;
	uint8_t *dstpix = (uint8_t *) dst->pixels;
	uint32_t TH0, TH1, TH2, TH3, TW0, TW1, TW2, TW3;
	uint32_t pixel;

	for (; scl.sy < scl.lines; scl.sy++) {
		TH0 = ((scl.oy << 2) * dstpitch);
		TH1 = TH0 + dstpitch;
		TH2 = TH1 + dstpitch;
		TH3 = TH2 + dstpitch;

		scl.ox = 0;
		/* loop per l'intera larghezza dell'immagine */
		for (scl.sx = scl.startx; scl.sx < scl.rows; scl.sx++) {
			pixel = palette[screen_index[scl.sy][scl.sx]];

			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = scl.ox << 3;
					TW1 = TW0 + 2;
					TW2 = TW0 + 4;
					TW3 = TW0 + 6;
					put_pixel(uint16_t, TH0, TW0);
					put_pixel(uint16_t, TH0, TW1);
					put_pixel(uint16_t, TH0, TW2);
					put_pixel(uint16_t, TH0, TW3);
					put_pixel(uint16_t, TH1, TW0);
					put_pixel(uint16_t, TH1, TW1);
					put_pixel(uint16_t, TH1, TW2);
					put_pixel(uint16_t, TH1, TW3);
					put_pixel(uint16_t, TH2, TW0);
					put_pixel(uint16_t, TH2, TW1);
					put_pixel(uint16_t, TH2, TW2);
					put_pixel(uint16_t, TH2, TW3);
					put_pixel(uint16_t, TH3, TW0);
					put_pixel(uint16_t, TH3, TW1);
					put_pixel(uint16_t, TH3, TW2);
					put_pixel(uint16_t, TH3, TW3);
					break;
				case 24:
					TW0 = X3((scl.ox << 2));
					TW1 = TW0 + 3;
					TW2 = TW0 + 6;
					TW3 = TW0 + 9;
					put_pixel(int, TH0, TW0);
					put_pixel(int, TH0, TW1);
					put_pixel(int, TH0, TW2);
					put_pixel(int, TH0, TW3);
					put_pixel(int, TH1, TW0);
					put_pixel(int, TH1, TW1);
					put_pixel(int, TH1, TW2);
					put_pixel(int, TH1, TW3);
					put_pixel(int, TH2, TW0);
					put_pixel(int, TH2, TW1);
					put_pixel(int, TH2, TW2);
					put_pixel(int, TH2, TW3);
					put_pixel(int, TH3, TW0);
					put_pixel(int, TH3, TW1);
					put_pixel(int, TH3, TW2);
					put_pixel(int, TH3, TW3);
					break;
				case 32:
					TW0 = (scl.ox << 4);
					TW1 = TW0 + 4;
					TW2 = TW0 + 8;
					TW3 = TW0 + 12;
					put_pixel(uint32_t, TH0, TW0);
					put_pixel(uint32_t, TH0, TW1);
					put_pixel(uint32_t, TH0, TW2);
					put_pixel(uint32_t, TH0, TW3);
					put_pixel(uint32_t, TH1, TW0);
					put_pixel(uint32_t, TH1, TW1);
					put_pixel(uint32_t, TH1, TW2);
					put_pixel(uint32_t, TH1, TW3);
					put_pixel(uint32_t, TH2, TW0);
					put_pixel(uint32_t, TH2, TW1);
					put_pixel(uint32_t, TH2, TW2);
					put_pixel(uint32_t, TH2, TW3);
					put_pixel(uint32_t, TH3, TW0);
					put_pixel(uint32_t, TH3, TW1);
					put_pixel(uint32_t, TH3, TW2);
					put_pixel(uint32_t, TH3, TW3);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
