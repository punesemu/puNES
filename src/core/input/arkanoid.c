/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "conf.h"
#include "input/arkanoid.h"
#include "input/mouse.h"

enum _arkanoid_enum {
	ark_start_x = 98,
	ark_stop_x = 242,
	ark_rows = ark_stop_x - ark_start_x
};

struct _arkanoid {
	int x, button;
} arkanoid[PORT_BASE];

void input_init_arkanoid(void) {
	BYTE i;

	for (i = 0; i <= PORT2; i++) {
		arkanoid[i].x = 98;
		arkanoid[i].button = 0;
	}
}
void input_wr_arkanoid(BYTE *value, BYTE nport) {
	static const float ratio = (float) ark_rows / (float) ark_stop_x;

	if (nport > PORT2) {
		return;
	}

	if ((r4016.value & 0x01) && !((*value) & 0x01)) {;
		int x, y;

		input_read_mouse_coords(&x, &y);

		arkanoid[nport].x = ark_start_x + ((float) x * ratio);

		if (arkanoid[nport].x < ark_start_x) {
			arkanoid[nport].x = ark_start_x;
		}

		if (arkanoid[nport].x > ark_stop_x) {
			arkanoid[nport].x = ark_stop_x;
		}

		arkanoid[nport].x = ~arkanoid[nport].x;
		arkanoid[nport].button = gmouse.left;
	}
}
void input_rd_arkanoid(BYTE *value, BYTE nport) {
	if (nport > PORT2) {
		return;
	}

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		if (nport == PORT1) {
			(*value) |= (arkanoid[0].button << 1);
			return;
		}

		if (nport == PORT2) {
			(*value) |= (arkanoid[0].x & 0x80) >> 6;
			arkanoid[0].x = (arkanoid[0].x << 1) & 0xFF;
			return;
		}
	}

	(*value) |= (arkanoid[nport].x & 0x80) >> 3;
	arkanoid[nport].x = (arkanoid[nport].x << 1) & 0xFF;
	(*value) |= (arkanoid[nport].button << 3);
}
