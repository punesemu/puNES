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
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

struct _m226 {
	BYTE reg[2];
	BYTE reset;
} m226;
struct _m226tmp {
	BYTE reset_based;
} m226tmp;

void map_init_226(BYTE model) {
	EXTCL_CPU_WR_MEM(226);
	EXTCL_SAVE_MAPPER(226);
	mapper.internal_struct[0] = (BYTE *)&m226;
	mapper.internal_struct_size[0] = sizeof(m226);

	// 42-in-1 (Reset Based) [U][p1][!].unf
	m226tmp.reset_based = model == MAP233;

	if (m226tmp.reset_based && (info.reset == RESET)) {
		m226.reg[0] = 0;
		m226.reg[1] = 0;
		m226.reset ^= 1;
	} else {
		memset(&m226, 0x00, sizeof(m226));
	}
	extcl_cpu_wr_mem_226(0, m226.reg[0]);
}
void extcl_cpu_wr_mem_226(WORD address, BYTE value) {
	BYTE bank;

	m226.reg[address & 0x0001] = value;

	if (m226tmp.reset_based) {
		bank = (m226.reg[0] & 0x1F) | (m226.reset << 5) | ((m226.reg[1] << 6) & 0x40);
	} else {
		bank = (m226.reg[0] & 0x1F) | ((m226.reg[0] >> 2) & 0x20) | ((m226.reg[1] << 6) & 0x40);
	}

	if (m226.reg[0] & 0x20) {
		value = bank;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = bank >> 1;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();

	if (m226.reg[0] & 0x40) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
BYTE extcl_save_mapper_226(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m226.reg);
	if (save_slot.version >= 26) {
		save_slot_ele(mode, slot, m226.reset);
	}

	return (EXIT_OK);
}
