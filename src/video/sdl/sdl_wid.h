/*
 * sdl_wid.h
 *
 *  Created on: 12/gen/2015
 *      Author: fhorse
 */

#ifndef SDL_WID_H_
#define SDL_WID_H_

static char SDL_windowhack[50];

#define sdl_wid()\
	if (info.gui) {\
		SDL_putenv(SDL_windowhack);\
	}

#endif /* SDL_WID_H_ */
