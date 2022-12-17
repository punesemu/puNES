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

#ifndef OVERSCAN_H_
#define OVERSCAN_H_

#include "common.h"

enum overscan_limit {
	OVERSCAN_BORDERS_MIN = 0, OVERSCAN_BORDERS_MAX = 17
};

typedef struct _overscan_borders {
	BYTE up;
	BYTE down;
	BYTE left;
	BYTE right;
} _overscan_borders;
typedef struct _overscan {
	BYTE enabled;
	_overscan_borders *borders;
} _overscan;

extern _overscan_borders overscan_borders[2];
extern _overscan overscan;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE overscan_set_mode(BYTE mode);

#undef EXTERNC

#endif /* OVERSCAN_H_ */
