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
#include "input.h"
#include "input/family_basic_keyboard.h"
#include "tape_data_recorder.h"
#include "conf.h"
#include "gui.h"

void input_wr_family_basic_keyboard(BYTE *value, UNUSED(BYTE nport)) {
	BYTE column;

	// 7  bit  0
	// ---- ----
	// xxxx xKCR
	//       |||
	//       ||+-- Reset the keyboard to the first row.
	//       |+--- Select column, row is incremented if this bit goes from high to low.
	//       +---- Enable keyboard matrix (if 0, all voltages inside the keyboard will be 5V, reading back as logical 0 always)
	generic_keyboard.enable = (*value) & 0x04;
	column = ((*value) & 0x02) >> 1;
	if (generic_keyboard.enable) {
		if ((*value) & 0x01) {
			generic_keyboard.row = 0;
			gui_nes_keyboard_paste_event();
		} else if (generic_keyboard.column && !column) {
			generic_keyboard.row = (generic_keyboard.row + 1) % 10;
		}
		generic_keyboard.column = column;
	}

	// tape data recorder
	// 7  bit  0
	// ---- ----
	// xxxx xExS
	//       | |
	//       | +- 1-bit DAC audio to audio cassette
	//       +--- When 0, force audio readback to always read as binary 0 (5V)
	tape_data_recorder.in = ((*value) & 0x01);
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
		if (generic_keyboard.column) {
			state = (generic_keyboard.data[generic_keyboard.row] & 0xF0) >> 3;
		} else {
			state = (generic_keyboard.data[generic_keyboard.row] & 0x0F) << 1;
		}
		(*value) = ((*value) & 0xE1) | (generic_keyboard.enable ? state ^ 0x1E : 0x00);
	} else {
		// r4016
		// 7  bit  0
		// ---- ----
		// xxxx xxAx
		//        |
		//        +-- 1-bit ADC audio from audio cassette
		(*value) = ((*value) & 0xFD) | (generic_keyboard.enable ? ((tape_data_recorder.out & 0x01) << 1) : 0x00);
	}
}

void input_add_event_family_basic_keyboard(UNUSED(BYTE index)) {
	WORD a, b;

	for (a = 0; a < nes_keyboard.rows; a++) {
		generic_keyboard.data[a] = 0;
		for (b = 0; b < nes_keyboard.columns; b++) {
			if (nes_keyboard.matrix[(a * nes_keyboard.columns) + b] & 0x80) {
				generic_keyboard.data[a] |= (1 << b);
			}
		}
	}
}
