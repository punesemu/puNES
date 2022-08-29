/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "input/family_basic_keyboard.h"
#include "conf.h"
#include "gui.h"

struct _family_basic_keyboard {
	BYTE row;
	BYTE column;
	BYTE enable;
	BYTE state;
	BYTE data[10];
} family_basic_keyboard;

void input_init_family_basic_keyboard(void) {
	memset(&family_basic_keyboard, 0x00, sizeof(family_basic_keyboard));
}

void input_wr_family_basic_keyboard(BYTE *value, UNUSED(BYTE nport)) {
	BYTE column;

	// 7  bit  0
	// ---- ----
	// xxxx xKCR
	//       |||
	//       ||+-- Reset the keyboard to the first row.
	//       |+--- Select column, row is incremented if this bit goes from high to low.
	//       +---- Enable keyboard matrix (if 0, all voltages inside the keyboard will be 5V, reading back as logical 0 always)
	family_basic_keyboard.enable = (*value) & 0x04;
	column = ((*value) & 0x02) >> 1;
	if (family_basic_keyboard.enable) {
		if ((*value) & 0x01) {
			family_basic_keyboard.row = 0;
			gui_nes_keyboard_paste_event();
		} else if (family_basic_keyboard.column && !column) {
			family_basic_keyboard.row = (family_basic_keyboard.row + 1) % 10;
		}
		family_basic_keyboard.column = column;
	}
}
void input_rd_family_basic_keyboard(BYTE *value, BYTE nport, UNUSED(BYTE shift)) {
	if (nport & 0x01) {
		BYTE state = 0;

		// r4017
		// 7  bit  0
		// ---- ----
		// xxxK KKKx
		//    | |||
		//    +-+++--- Receive key status of currently selected row/column.
		if (family_basic_keyboard.column) {
			state = (family_basic_keyboard.data[family_basic_keyboard.row] & 0xF0) >> 3;
		} else {
			state = (family_basic_keyboard.data[family_basic_keyboard.row] & 0x0F) << 1;
		}
		(*value) = family_basic_keyboard.enable ? state ^ 0x1E : 0x00;
	} else {
		// r4016
	}
}

void input_add_event_family_basic_keyboard(UNUSED(BYTE index)) {
	WORD a, b;

	for (a = 0; a < nes_keyboard.rows; a++) {
		family_basic_keyboard.data[a] = 0;
		for (b = 0; b < nes_keyboard.columns; b++) {
			if (nes_keyboard.keys[(a * nes_keyboard.columns) + b] & 0x80) {
				family_basic_keyboard.data[a] |= (1 << b);
			}
		}
	}
}
