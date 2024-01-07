/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include "input.h"
#include "input/subor_keyboard.h"
#include "tape_data_recorder.h"
#include "conf.h"
#include "gui.h"

void input_wr_subor_keyboard(UNUSED(BYTE nidx), const BYTE *value, UNUSED(BYTE nport)) {
	BYTE column = 0;

	generic_keyboard.enable = (*value) & 0x04;
	column = ((*value) & 0x02) >> 1;
	if (generic_keyboard.enable) {
		if ((*value) & 0x01) {
			generic_keyboard.row = 0;
			gui_nes_keyboard_paste_event();
		} else if (generic_keyboard.column && !column) {
			generic_keyboard.row = (generic_keyboard.row + 1) % 13;
		}
		generic_keyboard.column = column;
	}
	tape_data_recorder.in = ((*value) & 0x02) >> 1;
}
void input_rd_subor_keyboard(UNUSED(BYTE nidx), BYTE *value, BYTE nport, UNUSED(BYTE shift)) {
	if (nport & 0x01) {
		BYTE state = 0;

		if (generic_keyboard.column) {
			state = (generic_keyboard.data[generic_keyboard.row] & 0xF0) >> 3;
		} else {
			state = (generic_keyboard.data[generic_keyboard.row] & 0x0F) << 1;
		}
		(*value) = ((*value) & 0xE1) | (generic_keyboard.enable ? state ^ 0x1E : 0x00);
	} else {
		(*value) = ((*value) & 0xFB) | ((tape_data_recorder.out & 0x01) << 2);
	}
}

void input_add_event_subor_keyboard(UNUSED(BYTE index)) {
	WORD a = 0, b = 0;

	for (a = 0; a < nes_keyboard.rows; a++) {
		generic_keyboard.data[a] = 0;
		for (b = 0; b < nes_keyboard.columns; b++) {
			if (nes_keyboard.matrix[(a * nes_keyboard.columns) + b] & 0x80) {
				generic_keyboard.data[a] |= (1 << b);
			}
		}
	}
	// From https://problemkaputt.de/everynes.htm :
	// > The 32-in-1 menu also checks Bit4 in Row 9, if that bit is zero then it does additionally read row 0Ah..0Ch.
	// > Aside from the menu, most or all games in the 32-in-1 cartridge don't seem to use that extra rows though.
	// sempre nel 32-in-1, nell "English editor" se il bit è sempre a 1 inizia a scrivere ripetutamente l'ultimo carattere
	// nel suo buffer.
	if (cfg->input.vk_subor_extended_mode) {
		generic_keyboard.data[9] |= 0x10;
	}
}
