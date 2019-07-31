/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef IRQL2F_H_
#define IRQL2F_H_

#include "cpu.h"
#include "ppu.h"

enum {
	IRQL2F_INFRAME = 0x40,
	IRQL2F_PENDING = 0x80
};

typedef struct {
	BYTE present;
	BYTE enable;
	BYTE counter;
	BYTE scanline;
	WORD frame_x;
	BYTE delay;
	BYTE in_frame;
	BYTE pending;
} _irql2f;

_irql2f irql2f;

void irql2f_tick(void);

#endif /* IRQL2F_H_ */
