/*
 * ntsc.h
 *
 *  Created on: 18/ago/2010
 *      Author: fhorse
 */

#ifndef NTSC_H_
#define NTSC_H_

#include <SDL.h>
#include "common.h"
#include "nes_ntsc.h"
#include "palette.h"

enum { COMPOSITE, SVIDEO, RGBMODE };

void ntscSurface(WORD *screen, WORD **screeIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor);
BYTE ntscInit(BYTE effect, BYTE color, BYTE *paletteBase, BYTE *paletteIN, BYTE *paletteOUT);
void ntscQuit(void);
void ntscSet(BYTE effect, BYTE color, BYTE *paletteBase, BYTE *paletteIN, BYTE *paletteOUT);

#endif /* NTSC_H_ */
