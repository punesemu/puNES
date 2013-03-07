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

void scale_surface(WORD *screen, WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch,
        void *pix, WORD rows, WORD lines, BYTE factor) {
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
		scale_surface1x(screen_index, palette, bpp, pitch, pix);
	}
	/*
	else if (factor == 2) {
		scale_surface2x(screen_index, palette, dst);
	} else if (factor == 3) {
		scale_surface3x(screen_index, palette, dst);
	} else if (factor == 4) {
		scale_surface4x(screen_index, palette, dst);
	}
	*/
}
void scale_surface1x(WORD **screen_index, uint32_t *palette, BYTE bpp, uint32_t pitch, void *pix) {
	const uint32_t dstpitch = pitch;
	uint8_t *dstpix = (uint8_t *) pix;
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
			switch (bpp) {
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
