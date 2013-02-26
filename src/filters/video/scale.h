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

void scale_surface(WORD *screen, WORD **screen_index, Uint32 *palette, SDL_Surface *dst, WORD rows,
		WORD lines, BYTE factor);
void scale_surface1x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst);
void scale_surface2x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst);
void scale_surface3x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst);
void scale_surface4x(WORD **screen_index, uint32_t *palette, SDL_Surface *dst);

#endif /* SCALE_H_ */
