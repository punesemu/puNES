/*
 * scale2x.h
 *
 *  Created on: 21/mag/2010
 *      Author: fhorse
 */

#ifndef SCALE2X_H_
#define SCALE2X_H_

#include <SDL.h>
#include "common.h"
#include "palette.h"

void scaleNx(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor);
void scale2x(WORD **screenIndex, Uint32 *palette, SDL_Surface *dst);
void scale3x(WORD **screenIndex, Uint32 *palette, SDL_Surface *dst);
void scale4x(WORD **screenIndex, Uint32 *palette, SDL_Surface *dst);

#endif /* SCALE2X_H_ */
