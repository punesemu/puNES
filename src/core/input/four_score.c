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

#include "input/four_score.h"
#include "vs_system.h"
#include "nes.h"
#include "info.h"

struct _four_score {
	BYTE signature;
} four_score[PORT_BASE];

void input_init_four_score(void) {
	for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
		nes[nesidx].c.input.fsindex[PORT1] = 0;
		four_score[PORT1].signature = 0x10;
		nes[nesidx].c.input.fsindex[PORT2] = 0;
		four_score[PORT2].signature = 0x20;
	}
}

BYTE input_wr_reg_four_score(BYTE nidx, BYTE value) {
	if (!(value & 0x01)) {
		nes[nidx].c.input.fsindex[PORT1] = 0;
		nes[nidx].c.input.fsindex[PORT2] = 0;
	}
	return (value);
}
BYTE input_rd_reg_four_score(BYTE nidx, BYTE openbus, BYTE nport) {
	BYTE value = 0;

	if (nes[nidx].c.input.fsindex[nport] < 8) {
		value = port[nport].data[nes[nidx].c.input.fsindex[nport] & 0x07];
		nes[nidx].c.input.fsindex[nport]++;
	} else if (nes[nidx].c.input.fsindex[nport] < 16) {
		value = port[nport + 2].data[nes[nidx].c.input.fsindex[nport] & 0x07];
		nes[nidx].c.input.fsindex[nport]++;
	} else if (nes[nidx].c.input.fsindex[nport] < 24) {
		value = (four_score[nport].signature >> (23 - nes[nidx].c.input.fsindex[nport])) & 0x01;
		nes[nidx].c.input.fsindex[nport]++;
	} else {
		value = 0x01;
	}
	return ((openbus & 0xE0) | value);
}

BYTE input_rd_reg_four_score_vs(BYTE nidx, BYTE openbus, BYTE nport) {
	BYTE index = nes[nidx].c.input.fsindex[nport] & 0x07;
	BYTE protection = FALSE;
	BYTE np = nport;
	BYTE value = 0;

	if (index == START) {
		index = SELECT;
		protection = vs_system.special_mode.type == VS_DS_Bungeling;
	} else if (index == SELECT) {
		index = START;
	} else if (info.mapper.expansion == EXP_VS_1P_R4017) {
		np ^= 0x01;
	}
	if (nes[nidx].c.input.fsindex[nport] < 8) {
		value = protection ? PRESSED : port[np].data[index];
		nes[nidx].c.input.fsindex[nport]++;
	} else if (nes[nidx].c.input.fsindex[nport] < 16) {
		value = protection ? PRESSED : port[np + 2].data[index];
		nes[nidx].c.input.fsindex[nport]++;
	} else if (nes[nidx].c.input.fsindex[nport] < 24) {
		value = (four_score[np].signature >> (23 - nes[nidx].c.input.fsindex[nport])) & 0x01;
		nes[nidx].c.input.fsindex[nport]++;
	} else {
		value = 0x01;
	}
	return ((openbus & 0xE0) | value);
}