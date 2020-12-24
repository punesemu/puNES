/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include "input/snes_mouse.h"
#include "input/mouse.h"

struct _snes_mouse {
	DBWORD latch;
	int lastx, lasty;
	BYTE sensitivity;
} snes_mouse[PORT_BASE];

void input_init_snes_mouse(void) {
	memset(&snes_mouse, 0x00, sizeof(snes_mouse));
}
void input_wr_snes_mouse(BYTE *value, BYTE nport) {
	if ((r4016.value & 0x01) && !((*value) & 0x01)) {
		BYTE dx = 0x00, dy = 0x00;
		int gx, gy;
		int x, y;

		input_read_mouse_coords(&x, &y);

		gx = x - snes_mouse[nport].lastx;
		gy = y - snes_mouse[nport].lasty;

		snes_mouse[nport].lastx = x;
		snes_mouse[nport].lasty = y;

		if (gx < 0) {
			dx = 0x80;
			gx = -gx;
		}
		if (gy < 0) {
			dy = 0x80;
			gy = -gy;
		}

		gx += gx >> (2 - snes_mouse[nport].sensitivity);
		gy += gy >> (2 - snes_mouse[nport].sensitivity);

		if (gx > 127) {
			gx = 127;
		}
		if (gy > 127) {
			gy = 127;
		}

		gx |= dx;
		gy |= dy;

		snes_mouse[nport].latch =
				(0x00 << 24) |
				(((gmouse.right << 7) | (gmouse.left << 6) | (snes_mouse[nport].sensitivity << 4) | 0x01) << 16) |
				((BYTE) gy << 8) |
				((BYTE) gx << 0);
	}
}
void input_rd_snes_mouse(BYTE *value, BYTE nport, UNUSED(BYTE shift)) {
	(*value) |= ((snes_mouse[nport].latch & 0x80000000) >> 31);

	if ((r4016.value & 0x01) && (++snes_mouse[nport].sensitivity > 2)) {
		snes_mouse[nport].sensitivity = 0;
	}

	snes_mouse[nport].latch <<= 1;
}
