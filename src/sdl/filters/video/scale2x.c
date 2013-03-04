/*
 * scale2x.c
 *
 *  Created on: 20/mag/2010
 *      Author: fhorse
 */

#include "scale2x.h"
#include "overscan.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define READINT24(x) ((x)[0] << 16 | (x)[1] << 8 | (x)[2])
#define WRITEINT24(x, i)\
	x[0] = i >> 16;\
	x[1] = (i >> 8) & 0xff;\
	x[2] = i & 0xff
#define X3(a) ((a << 1) + a)
#define SCALE2X()\
    if (B != H && D != F) {\
        E0 = D == B ? D : E;\
        E1 = B == F ? F : E;\
        E2 = D == H ? D : E;\
        E3 = H == F ? F : E;\
    } else {\
        E0 = E1 = E2 = E3 = E;\
    }
#define SCALE3X_A()\
    E0 = D == B ? D : E;\
    E1 = (D == B && E != C) || (B == F && E != A) ? B : E;\
    E2 = B == F ? F : E;\
    E3 = (D == B && E != G) || (D == H && E != A) ? D : E;\
    E4 = E;\
    E5 = (B == F && E != I) || (H == F && E != C) ? F : E;\
    E6 = D == H ? D : E;\
    E7 = (D == H && E != I) || (H == F && E != G) ? H : E;\
    E8 = H == F ? F : E;
#define SCALE3X_B()\
    E0 = E1 = E2 = E3 = E4 = E5 = E6 = E7 = E8 = E;
#define put_pixel(type, pixel, p0, p1)\
    *(type *) (dstpix + p0 + p1) = (type) palette[pixel]

struct _scl2x {
	WORD sx;
	WORD sy;
	WORD oy;
	WORD ox;
	WORD startx;
	WORD rows;
	WORD lines;
} scl2x;

/*
 * cio' che non utilizzo in questa funzione
 * e' il parametro WORD *screen.
 */
void scaleNx(WORD *screen, WORD **screen_index, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor) {

	scl2x.sx = 0;
	scl2x.sy = 0;
	scl2x.oy = 0;
	scl2x.lines = lines;
	scl2x.rows = rows;
	scl2x.startx = 0;

	if (overscan.enabled) {
		scl2x.sy += overscan.up;
		scl2x.lines += overscan.up;
		scl2x.rows += overscan.left;
		scl2x.startx = overscan.left;
	}

	if (factor == 1) {
		return;
	} else if (factor == 2) {
		scale2x(screen_index, palette, dst);
	} else if (factor == 3) {
		scale3x(screen_index, palette, dst);
	} else if (factor == 4) {
		scale4x(screen_index, palette, dst);
	}
}
void scale2x(WORD **screen_index, Uint32 *palette, SDL_Surface *dst) {
	const DBWORD dstpitch = dst->pitch;
	WORD E0, E1, E2, E3, B, D, E, F, H;
	Uint8 *dstpix = (Uint8 *) dst->pixels;
	Uint32 TH0, TH1, TH2, TH3, TH4;
	Uint32 TW0, TW1, TW2;

	/* lock della destinazione */
	//SDL_LockSurface(dst);
	for (; scl2x.sy < scl2x.lines; ++scl2x.sy) {
		TH0 = MAX(0, scl2x.sy - 1);
		TH1 = scl2x.sy;
		TH2 = MIN(scl2x.lines - 1, scl2x.sy + 1);
		TH3 = ((scl2x.oy << 1) * dstpitch);
		TH4 = TH3 + dstpitch;

		scl2x.ox = 0;

		for (scl2x.sx = scl2x.startx; scl2x.sx < scl2x.rows; ++scl2x.sx) {
			TW0 = MAX(0, scl2x.sx - 1);
			TW1 = scl2x.sx;
			TW2 = MIN(scl2x.rows - 1, scl2x.sx + 1);
			B = screen_index[TH0][TW1];
			D = screen_index[TH1][TW0];
			E = screen_index[TH1][TW1];
			F = screen_index[TH1][TW2];
			H = screen_index[TH2][TW1];
			SCALE2X()
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = (scl2x.ox << 2);
					TW1 = TW0 + 2;
					put_pixel(Uint16, E0, TH3, TW0);
					put_pixel(Uint16, E1, TH3, TW1);
					put_pixel(Uint16, E2, TH4, TW0);
					put_pixel(Uint16, E3, TH4, TW1);
					break;
				case 24:
					TW0 = X3((scl2x.ox << 1));
					TW1 = TW0 + 3;
					put_pixel(int, E0, TH3, TW0);
					put_pixel(int, E1, TH3, TW1);
					put_pixel(int, E2, TH4, TW0);
					put_pixel(int, E3, TH4, TW1);
					break;
				default:
					TW0 = (scl2x.ox << 3);
					TW1 = TW0 + 4;
					put_pixel(Uint32, E0, TH3, TW0);
					put_pixel(Uint32, E1, TH3, TW1);
					put_pixel(Uint32, E2, TH4, TW0);
					put_pixel(Uint32, E3, TH4, TW1);
					break;
			}
			scl2x.ox++;
		}
		scl2x.oy++;
	}
	/* unlock della destinazione */
	//SDL_UnlockSurface(dst);
}
void scale3x(WORD **screen_index, Uint32 *palette, SDL_Surface *dst) {
	const DBWORD dstpitch = dst->pitch;
	WORD A, B, C, D, E, F, G, H, I;
	WORD E0, E1, E2, E3, E4, E5, E6, E7, E8;
	Uint8 *dstpix = (Uint8 *) dst->pixels;
	Uint32 TH0, TH1, TH2, TH3, TH4, TH5;
	Uint32 TW0, TW1, TW2;

	/* lock della destinazione */
	//SDL_LockSurface(dst);
	for (; scl2x.sy < scl2x.lines; ++scl2x.sy) {
		TH0 = MAX(0, scl2x.sy - 1);
		TH1 = scl2x.sy;
		TH2 = MIN(scl2x.lines - 1, scl2x.sy + 1);
		TH3 = (X3(scl2x.oy) * dstpitch);
		TH4 = TH3 + dstpitch;
		TH5 = TH3 + (dstpitch << 1);

		scl2x.ox = 0;

		for (scl2x.sx = scl2x.startx; scl2x.sx < scl2x.rows; ++scl2x.sx) {
			TW0 = MAX(0, scl2x.sx - 1);
			TW1 = scl2x.sx;
			TW2 = MIN(scl2x.rows - 1, scl2x.sx + 1);
			B = screen_index[TH0][TW1];
			D = screen_index[TH1][TW0];
			E = screen_index[TH1][TW1];
			F = screen_index[TH1][TW2];
			H = screen_index[TH2][TW1];
			if (B != H && D != F) {
				A = screen_index[TH0][TW0];
				C = screen_index[TH0][TW2];
				G = screen_index[TH2][TW0];
				I = screen_index[TH2][TW2];
				SCALE3X_A()
			} else {
				SCALE3X_B()
			}
			switch (dst->format->BitsPerPixel) {
				case 15:
				case 16:
					TW0 = (X3(scl2x.ox) << 1);
					TW1 = TW0 + 2;
					TW2 = TW0 + 4;
					put_pixel(Uint16, E0, TH3, TW0);
					put_pixel(Uint16, E1, TH3, TW1);
					put_pixel(Uint16, E2, TH3, TW2);
					put_pixel(Uint16, E3, TH4, TW0);
					put_pixel(Uint16, E4, TH4, TW1);
					put_pixel(Uint16, E5, TH4, TW2);
					put_pixel(Uint16, E6, TH5, TW0);
					put_pixel(Uint16, E7, TH5, TW1);
					put_pixel(Uint16, E8, TH5, TW2);
					break;
				case 24:
					TW0 = X3((X3(scl2x.ox)));
					TW1 = TW0 + 3;
					TW2 = TW0 + 6;
					put_pixel(int, E0, TH3, TW0);
					put_pixel(int, E1, TH3, TW1);
					put_pixel(int, E2, TH3, TW2);
					put_pixel(int, E3, TH4, TW0);
					put_pixel(int, E4, TH4, TW1);
					put_pixel(int, E5, TH4, TW2);
					put_pixel(int, E6, TH5, TW0);
					put_pixel(int, E7, TH5, TW1);
					put_pixel(int, E8, TH5, TW2);
					break;
				default:
					TW0 = (X3(scl2x.ox) << 2);
					TW1 = TW0 + 4;
					TW2 = TW0 + 8;
					put_pixel(Uint32, E0, TH3, TW0);
					put_pixel(Uint32, E1, TH3, TW1);
					put_pixel(Uint32, E2, TH3, TW2);
					put_pixel(Uint32, E3, TH4, TW0);
					put_pixel(Uint32, E4, TH4, TW1);
					put_pixel(Uint32, E5, TH4, TW2);
					put_pixel(Uint32, E6, TH5, TW0);
					put_pixel(Uint32, E7, TH5, TW1);
					put_pixel(Uint32, E8, TH5, TW2);
					break;
			}
			scl2x.ox++;
		}
		scl2x.oy++;
	}
	/* unlock della destinazione */
	//SDL_UnlockSurface(dst);
}
void scale4x(WORD **screen_index, Uint32 *palette, SDL_Surface *dst) {
	SDL_Surface *buffer;
	WORD x, y, width, height;
	DBWORD srcpitch, dstpitch = dst->pitch;
	Uint8 *srcpix, *dstpix = (Uint8 *) dst->pixels;
	Uint32 TH0, TH1, TH2, TH3, TH4;
	Uint32 TW0, TW1, TW2;

	buffer = SDL_CreateRGBSurface(dst->flags, dst->w >> 1, dst->h >> 1, dst->format->BitsPerPixel,
			dst->format->Rmask, dst->format->Gmask, dst->format->Bmask, dst->format->Amask);

	scale2x(screen_index, palette, buffer);

	srcpix = (Uint8 *) buffer->pixels;
	srcpitch = buffer->pitch;
	width = buffer->w;
	height = buffer->h;

	/* lock della destinazione */
	//SDL_LockSurface(dst);
	for (y = 0; y < height; ++y) {
		TH0 = (MAX(0, y - 1) * srcpitch);
		TH1 = (y * srcpitch);
		TH2 = (MIN(height - 1, y + 1) * srcpitch);
		TH3 = ((y << 1) * dstpitch);
		TH4 = TH3 + dstpitch;
		for (x = 0; x < width; ++x) {
			switch (buffer->format->BitsPerPixel) {
				case 15:
				case 16: {
					Uint16 E0, E1, E2, E3, B, D, E, F, H;
					TW0 = (MAX(0, x - 1) << 1);
					TW1 = (x << 1);
					TW2 = (MIN(width - 1, x + 1) << 1);
					B = *(Uint16 *) (srcpix + TH0 + TW1);
					D = *(Uint16 *) (srcpix + TH1 + TW0);
					E = *(Uint16 *) (srcpix + TH1 + TW1);
					F = *(Uint16 *) (srcpix + TH1 + TW2);
					H = *(Uint16 *) (srcpix + TH2 + TW1);
					SCALE2X()
					TW0 = (x << 2);
					TW1 = TW0 + 2;
					*(Uint16 *) (dstpix + TH3 + TW0) = E0;
					*(Uint16 *) (dstpix + TH3 + TW1) = E1;
					*(Uint16 *) (dstpix + TH4 + TW0) = E2;
					*(Uint16 *) (dstpix + TH4 + TW1) = E3;
					break;
				}
				case 24: {
					int E0, E1, E2, E3, B, D, E, F, H;
					TW0 = X3((MAX(0, x - 1)));
					TW1 = X3(x);
					TW2 = X3((MIN(width - 1, x + 1)));
					B = READINT24(srcpix + TH0 + TW1);
					D = READINT24(srcpix + TH1 + TW0);
					E = READINT24(srcpix + TH1 + TW1);
					F = READINT24(srcpix + TH1 + TW2);
					H = READINT24(srcpix + TH2 + TW1);
					SCALE2X()
					TW0 = X3((x << 1));
					TW1 = TW0 + 3;
					WRITEINT24((dstpix + TH3 + TW0), E0);
					WRITEINT24((dstpix + TH3 + TW1), E1);
					WRITEINT24((dstpix + TH4 + TW0), E2);
					WRITEINT24((dstpix + TH4 + TW1), E3);
					break;
				}
				default: {
					Uint32 E0, E1, E2, E3, B, D, E, F, H;
					TW0 = (MAX(0, x - 1) << 2);
					TW1 = (x << 2);
					TW2 = (MIN(width - 1, x + 1) << 2);
					B = *(Uint32 *) (srcpix + TH0 + TW1);
					D = *(Uint32 *) (srcpix + TH1 + TW0);
					E = *(Uint32 *) (srcpix + TH1 + TW1);
					F = *(Uint32 *) (srcpix + TH1 + TW2);
					H = *(Uint32 *) (srcpix + TH2 + TW1);
					SCALE2X()
					TW0 = (x << 3);
					TW1 = TW0 + 4;
					*(Uint32 *) (dstpix + TH3 + TW0) = E0;
					*(Uint32 *) (dstpix + TH3 + TW1) = E1;
					*(Uint32 *) (dstpix + TH4 + TW0) = E2;
					*(Uint32 *) (dstpix + TH4 + TW1) = E3;
					break;
				}
			}
		}
	}
	/* unlock della destinazione */
	//SDL_UnlockSurface(dst);
	/* distruggo la superficie temporanea */
	SDL_FreeSurface(buffer);
}
