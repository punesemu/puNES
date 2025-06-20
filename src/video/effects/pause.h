/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef PAUSE_H_
#define PAUSE_H_

#include "common.h"

enum _pause_effect_misc {
	PAUSE_EFFECT_FRAMES = 10
};

typedef struct _pause_effect {
	void *palette;
	void *ntsc;
	int frames;
} _pause_effect;

extern _pause_effect pause_effect;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE pause_init(void);
EXTERNC void pause_quit(void);

#undef EXTERNC

#endif /* PAUSE_H_ */
