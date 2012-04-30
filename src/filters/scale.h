/*
 * scale.h
 *
 *  Created on: 22/mag/2010
 *      Author: fhorse
 */

#ifndef SCALE_H_
#define SCALE_H_

#include <SDL.h>
#include "common.h"
#include "palette.h"

void scaleSurface(WORD *screen, WORD **screenIndex, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor);

#endif /* SCALE_H_ */
