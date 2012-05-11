/*
 * scale.c
 *
 *  Created on: 22/mag/2010
 *      Author: fhorse
 */

#include "scale.h"
#include "overscan.h"

#define X3(a) ((a << 1) + a)
#define PUTPIXEL(type, p0, p1)\
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

void scaleSurface(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
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
		scaleSurface1x(screenIndex, palette, dst);
	} else if (factor == 2) {
		scaleSurface2x(screenIndex, palette, dst);
	} else if (factor == 3) {
		scaleSurface3x(screenIndex, palette, dst);
	} else if (factor == 4) {
		scaleSurface4x(screenIndex, palette, dst);
	}
}
void scaleSurface1x(WORD **screenIndex, uint32_t *palette, SDL_Surface *dst) {
	const uint32_t dstpitch = dst->pitch;
	uint8_t *dstpix = (uint8_t *) dst->pixels;
	uint32_t TH0, TW0;
	uint32_t pixel;

	for (; scl.sy < scl.lines; scl.sy++) {
		TH0 = (scl.oy * dstpitch);
		scl.ox = 0;
		/* loop per l'intera larghezza dell'immagine */
		for (scl.sx = scl.startx; scl.sx < scl.rows; scl.sx++) {
			pixel = palette[screenIndex[scl.sy][scl.sx]];
			/*
			 * converto il colore nel formato corretto di visualizzazione
			 * e riempio un rettangolo delle dimensioni del fattore di scala
			 * alle coordinate corrette.
			 */
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = (scl.ox << 1);
					PUTPIXEL(uint16_t, TH0, TW0);
					break;
				case 24:
					TW0 = (scl.ox << 1) + 1;
					PUTPIXEL(int, TH0, TW0);
					break;
				case 32:
					TW0 = (scl.ox << 2);
					PUTPIXEL(uint32_t, TH0, TW0);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
void scaleSurface2x(WORD **screenIndex, uint32_t *palette, SDL_Surface *dst) {
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
			pixel = palette[screenIndex[scl.sy][scl.sx]];

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
					PUTPIXEL(uint16_t, TH0, TW0);
					PUTPIXEL(uint16_t, TH0, TW1);
					PUTPIXEL(uint16_t, TH1, TW0);
					PUTPIXEL(uint16_t, TH1, TW1);
					break;
				case 24:
					TW0 = X3((scl.ox << 1));
					TW1 = TW0 + 3;
					PUTPIXEL(int, TH0, TW0);
					PUTPIXEL(int, TH0, TW1);
					PUTPIXEL(int, TH1, TW0);
					PUTPIXEL(int, TH1, TW1);
					break;
				case 32:
					TW0 = (scl.ox << 3);
					TW1 = TW0 + 4;
					PUTPIXEL(uint32_t, TH0, TW0);
					PUTPIXEL(uint32_t, TH0, TW1);
					PUTPIXEL(uint32_t, TH1, TW0);
					PUTPIXEL(uint32_t, TH1, TW1);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
void scaleSurface3x(WORD **screenIndex, uint32_t *palette, SDL_Surface *dst) {
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
			pixel = palette[screenIndex[scl.sy][scl.sx]];

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
					PUTPIXEL(uint16_t, TH0, TW0);
					PUTPIXEL(uint16_t, TH0, TW1);
					PUTPIXEL(uint16_t, TH0, TW2);
					PUTPIXEL(uint16_t, TH1, TW0);
					PUTPIXEL(uint16_t, TH1, TW1);
					PUTPIXEL(uint16_t, TH1, TW2);
					PUTPIXEL(uint16_t, TH2, TW0);
					PUTPIXEL(uint16_t, TH2, TW1);
					PUTPIXEL(uint16_t, TH2, TW2);
					break;
				case 24:
					TW0 = X3((X3(scl.ox)));
					TW1 = TW0 + 3;
					TW2 = TW0 + 6;
					PUTPIXEL(int, TH0, TW0);
					PUTPIXEL(int, TH0, TW1);
					PUTPIXEL(int, TH0, TW2);
					PUTPIXEL(int, TH1, TW0);
					PUTPIXEL(int, TH1, TW1);
					PUTPIXEL(int, TH1, TW2);
					PUTPIXEL(int, TH2, TW0);
					PUTPIXEL(int, TH2, TW1);
					PUTPIXEL(int, TH2, TW2);
					break;
				case 32:
					TW0 = (X3(scl.ox) << 2);
					TW1 = TW0 + 4;
					TW2 = TW0 + 8;
					PUTPIXEL(uint32_t, TH0, TW0);
					PUTPIXEL(uint32_t, TH0, TW1);
					PUTPIXEL(uint32_t, TH0, TW2);
					PUTPIXEL(uint32_t, TH1, TW0);
					PUTPIXEL(uint32_t, TH1, TW1);
					PUTPIXEL(uint32_t, TH1, TW2);
					PUTPIXEL(uint32_t, TH2, TW0);
					PUTPIXEL(uint32_t, TH2, TW1);
					PUTPIXEL(uint32_t, TH2, TW2);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
void scaleSurface4x(WORD **screenIndex, uint32_t *palette, SDL_Surface *dst) {
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
			pixel = palette[screenIndex[scl.sy][scl.sx]];

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
					PUTPIXEL(uint16_t, TH0, TW0);
					PUTPIXEL(uint16_t, TH0, TW1);
					PUTPIXEL(uint16_t, TH0, TW2);
					PUTPIXEL(uint16_t, TH0, TW3);
					PUTPIXEL(uint16_t, TH1, TW0);
					PUTPIXEL(uint16_t, TH1, TW1);
					PUTPIXEL(uint16_t, TH1, TW2);
					PUTPIXEL(uint16_t, TH1, TW3);
					PUTPIXEL(uint16_t, TH2, TW0);
					PUTPIXEL(uint16_t, TH2, TW1);
					PUTPIXEL(uint16_t, TH2, TW2);
					PUTPIXEL(uint16_t, TH2, TW3);
					PUTPIXEL(uint16_t, TH3, TW0);
					PUTPIXEL(uint16_t, TH3, TW1);
					PUTPIXEL(uint16_t, TH3, TW2);
					PUTPIXEL(uint16_t, TH3, TW3);
					break;
				case 24:
					TW0 = X3((scl.ox << 2));
					TW1 = TW0 + 3;
					TW2 = TW0 + 6;
					TW3 = TW0 + 9;
					PUTPIXEL(int, TH0, TW0);
					PUTPIXEL(int, TH0, TW1);
					PUTPIXEL(int, TH0, TW2);
					PUTPIXEL(int, TH0, TW3);
					PUTPIXEL(int, TH1, TW0);
					PUTPIXEL(int, TH1, TW1);
					PUTPIXEL(int, TH1, TW2);
					PUTPIXEL(int, TH1, TW3);
					PUTPIXEL(int, TH2, TW0);
					PUTPIXEL(int, TH2, TW1);
					PUTPIXEL(int, TH2, TW2);
					PUTPIXEL(int, TH2, TW3);
					PUTPIXEL(int, TH3, TW0);
					PUTPIXEL(int, TH3, TW1);
					PUTPIXEL(int, TH3, TW2);
					PUTPIXEL(int, TH3, TW3);
					break;
				case 32:
					TW0 = (scl.ox << 4);
					TW1 = TW0 + 4;
					TW2 = TW0 + 8;
					TW3 = TW0 + 12;
					PUTPIXEL(uint32_t, TH0, TW0);
					PUTPIXEL(uint32_t, TH0, TW1);
					PUTPIXEL(uint32_t, TH0, TW2);
					PUTPIXEL(uint32_t, TH0, TW3);
					PUTPIXEL(uint32_t, TH1, TW0);
					PUTPIXEL(uint32_t, TH1, TW1);
					PUTPIXEL(uint32_t, TH1, TW2);
					PUTPIXEL(uint32_t, TH1, TW3);
					PUTPIXEL(uint32_t, TH2, TW0);
					PUTPIXEL(uint32_t, TH2, TW1);
					PUTPIXEL(uint32_t, TH2, TW2);
					PUTPIXEL(uint32_t, TH2, TW3);
					PUTPIXEL(uint32_t, TH3, TW0);
					PUTPIXEL(uint32_t, TH3, TW1);
					PUTPIXEL(uint32_t, TH3, TW2);
					PUTPIXEL(uint32_t, TH3, TW3);
					break;
			}
			scl.ox++;
		}
		scl.oy++;
	}
}
