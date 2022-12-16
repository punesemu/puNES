/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef TV_NOISE_H_
#define TV_NOISE_H_

#include "common.h"

typedef struct _turn_off_effect {
	void *palette;
	void *ntsc;
} _turn_off_effect;

extern _turn_off_effect turn_off_effect;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE tv_noise_init(void);
EXTERNC void tv_noise_quit(void);
EXTERNC void tv_noise_effect(void);

#undef EXTERNC

#endif /* TV_NOISE_H_ */
