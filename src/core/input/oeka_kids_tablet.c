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

#include <string.h>
#include "input/oeka_kids_tablet.h"
#include "input/mouse.h"

struct _oeka_kids_tablet {
	DBWORD latch;
	BYTE value;
} oeka_kids_tablet;

void input_init_oeka_kids_tablet(void) {
	memset(&oeka_kids_tablet, 0x00, sizeof(oeka_kids_tablet));
}
void input_wr_oeka_kids_tablet(const BYTE *value, UNUSED(BYTE nport)) {
	if (!((*value) & 0x01)) {
		int x, y;

		oeka_kids_tablet.latch = oeka_kids_tablet.value = 0;

		input_read_mouse_coords(&x, &y);

		if (gmouse.right) {
			oeka_kids_tablet.latch |= 0x02;
		} else {
			oeka_kids_tablet.latch |= 0x00;
		}

		if (gmouse.left) {
			oeka_kids_tablet.latch |= 0x01;
		}

		x = (int)(((float)x * (240.0f / 256.0f)) + 8);
		if (x < 0) {
			x = 0;
		}
		if (x > 255) {
			x = 255;
		}

		y = (int)(((float)y * (256.0f / 240.0f)) - 14);
		if (y < 0) {
			y = 0;
		}
		if (y > 255) {
			y = 255;
		}

		oeka_kids_tablet.latch |= (x << 10) | (y << 2);
	} else {
		if (!(r4016.value & 0x02) && ((*value) & 0x02)) {
			oeka_kids_tablet.latch <<= 1;
		}

		if (!((*value) & 0x02)) {
			oeka_kids_tablet.value = 0x04;
		} else {
			if (oeka_kids_tablet.latch & 0x40000) {
				oeka_kids_tablet.value = 0;
			} else {
				oeka_kids_tablet.value = 0x8;
			}
		}
	}
}
void input_rd_oeka_kids_tablet(BYTE *value, UNUSED(BYTE nport), UNUSED(BYTE shift)) {
	if (r4016.value & 0x03) {
		(*value) |= oeka_kids_tablet.value;
	}
}
