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

#ifndef XBRZ_WRAP_H_
#define XBRZ_WRAP_H_

#include "common.h"

#define XBRZ_NUM_SLICE 4

typedef struct _xbrz_wrap {
	int slice;
	BYTE factor;
	const WORD* src;
	uint32_t* trg;
	uint32_t* palette;
	int srcWidth;
	int srcHeight;
	int colFmt;
} _xbrz_wrap;

#endif /* XBRZ_WRAP_H_ */
