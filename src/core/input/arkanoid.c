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

#include "conf.h"
#include "input/arkanoid.h"
#include "input/mouse.h"

enum _arkanoid_enum {
	ark_start_x = 98,
	ark_stop_x = 242,
	ark_rows = ark_stop_x - ark_start_x
};

_arkanoid arkanoid[PORT_BASE];

void input_init_arkanoid(void) {
	int i;

	for (i = 0; i <= PORT2; i++) {
		arkanoid[i].x = arkanoid[i].rdx = 98;
		arkanoid[i].button = 0;
	}
}
void input_wr_arkanoid(UNUSED(BYTE nidx), const BYTE *value, BYTE nport) {
	static const float ratio = (float)ark_rows / (float)ark_stop_x;

	nport &= 0x01;

	if ((r4016.value & 0x01) && !((*value) & 0x01)) {
		int x = 0, y = 0;

		input_read_mouse_coords(&x, &y);

		arkanoid[nport].x = ark_start_x + (int)((float)x * ratio);

		if (arkanoid[nport].x < ark_start_x) {
			arkanoid[nport].x = ark_start_x;
		}

		if (arkanoid[nport].x > ark_stop_x) {
			arkanoid[nport].x = ark_stop_x;
		}

		arkanoid[nport].rdx = ~arkanoid[nport].x;
		arkanoid[nport].button = gmouse.left;
	}
}
void input_rd_arkanoid(UNUSED(BYTE nidx), BYTE *value, BYTE nport, UNUSED(BYTE shift)) {
	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		if ((nport & 0x01) == PORT1) {
			(*value) |= (arkanoid[0].button << 1);
			return;
		}

		if ((nport & 0x01) == PORT2) {
			(*value) |= (arkanoid[0].rdx & 0x80) >> 6;
			arkanoid[0].rdx = (arkanoid[0].rdx << 1) & 0xFF;
			return;
		}
	}

	(*value) |= (arkanoid[nport].rdx & 0x80) >> 3;
	arkanoid[nport].rdx = (arkanoid[nport].rdx << 1) & 0xFF;
	(*value) |= (arkanoid[nport].button << 3);
}
