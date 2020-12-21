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

#include "input/four_score.h"

struct _four_score {
	BYTE count;
	BYTE signature;
} four_score[PORT_BASE];

void input_init_four_score(void) {
	four_score[PORT1].count = 0;
	four_score[PORT1].signature = 0x10;
	four_score[PORT2].count = 0;
	four_score[PORT2].signature = 0x20;
}
BYTE input_wr_reg_four_score(BYTE value) {
	if (!(value & 0x01)) {
		four_score[PORT1].count = 0;
		four_score[PORT2].count = 0;
	}

	return (value);
}
BYTE input_rd_reg_four_score(BYTE openbus, BYTE nport) {
	BYTE value = 0;

	if (four_score[nport].count < 8) {
		value = port[nport].data[four_score[nport].count & 0x07];
	} else if (four_score[nport].count < 16) {
		value = port[nport + 2].data[four_score[nport].count & 0x07];
	} else if (four_score[nport].count < 24) {
		value = (four_score[nport].signature >> (23 - four_score[nport].count)) & 0x01;
	}

	if (++four_score[nport].count >= 32) {
		four_score[nport].count = 0;
	}

	return ((openbus & 0xE0) | value);
}
