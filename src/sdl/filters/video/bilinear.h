/*
 * bilinear.h
 *
 *  Created on: 14/gen/2012
 *      Author: fhorse
 */

#ifndef BILINEAR_H_
#define BILINEAR_H_

#include <SDL.h>
#include "common.h"

void bilinear(WORD *screen, WORD **screen_index, Uint32 *palette, SDL_Surface *dst, WORD rows,
        WORD lines, BYTE factor);

#endif /* BILINEAR_H_ */
