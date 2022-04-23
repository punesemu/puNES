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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

static const BYTE slots[4][4][2] = {
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x00, 1 }, { 0x00, 1 } },
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x20, 0 }, { 0x00, 1 } },
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x20, 0 }, { 0x40, 0 } },
	{ { 0x00, 0 }, { 0x20, 0 }, { 0x40, 0 }, { 0x60, 0 } }
};

struct _m235 {
	BYTE openbus;
} m235;
struct _m235tmp {
	BYTE type;
} m235tmp;

void map_init_235(void) {
	switch (info.prg.rom.banks_16k) {
		case 64:
			m235tmp.type = 0;
			break;
		case 128:
			m235tmp.type = 1;
			break;
		case 192:
			m235tmp.type = 2;
			break;
		case 256:
		default:
			m235tmp.type = 3;
			break;
	}

	EXTCL_CPU_WR_MEM(235);
	if (m235tmp.type != 3) {
		EXTCL_CPU_RD_MEM(235);
		EXTCL_SAVE_MAPPER(235);
		mapper.internal_struct[0] = (BYTE *)&m235;
		mapper.internal_struct_size[0] = sizeof(m235);

		info.mapper.extend_rd = TRUE;
	}

	if (info.reset >= HARD) {
		m235.openbus = 0;
		extcl_cpu_wr_mem_235(0x8000, 0x00);
	}
}
void extcl_cpu_wr_mem_235(WORD address, BYTE value) {
	BYTE bank = slots[m235tmp.type][(address >> 8) & 0x03][0] | (address & 0x1F);
	m235.openbus = slots[m235tmp.type][(address >> 8) & 0x03][1];

	if (address & 0x0800) {
		value = (bank << 1) | ((address >> 12) & 0x01);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = bank;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();

	if (address & 0x0400) {
		mirroring_SCR0();
	} else if (address & 0x2000) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
BYTE extcl_cpu_rd_mem_235(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (!m235.openbus || (address < 0x8000)) {
		return (openbus);
	}

	return (address >> 8);
}
BYTE extcl_save_mapper_235(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m235.openbus);

	return (EXIT_OK);
}
