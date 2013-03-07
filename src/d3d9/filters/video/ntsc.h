/*
 * ntsc.h
 *
 *  Created on: 18/ago/2010
 *      Author: fhorse
 */

#ifndef NTSC_H_
#define NTSC_H_

#include "common.h"
#include "nes_ntsc.h"
#include "palette.h"

enum { COMPOSITE, SVIDEO, RGBMODE };

//void ntsc_surface(WORD *screen, WORD **screeIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
//		WORD lines, BYTE factor);
BYTE ntsc_init(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out);
void ntsc_quit(void);
void ntsc_set(BYTE effect, BYTE color, BYTE *palette_base, BYTE *palette_in, BYTE *palette_out);

#endif /* NTSC_H_ */
