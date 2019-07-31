/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

void map_init_230(void) {
	EXTCL_CPU_WR_MEM(230);
	EXTCL_SAVE_MAPPER(230);
	mapper.internal_struct[0] = (BYTE *) &m230;
	mapper.internal_struct_size[0] = sizeof(m230);

	if (info.reset >= HARD) {
		m230.mode = 0;
	} else {
		m230.mode ^= 1;
	}

	if (m230.mode) {
		map_prg_rom_8k(2, 0, 0);
		map_prg_rom_8k(2, 2, 7);

		mirroring_V();
	} else {
		map_prg_rom_8k(2, 0, 8);
		map_prg_rom_8k(2, 2, info.prg.rom[0].max.banks_16k);
	}
}
void extcl_cpu_wr_mem_230(UNUSED(WORD address), BYTE value) {
	BYTE save = value;

	if (!m230.mode) {
		value = (save & 0x1F) + 0x08;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);

		value |= ((~save >> 5) & 0x01);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);

		if (save & 0x40) {
			mirroring_V();
		} else {
			mirroring_H();
		}
	} else {
		control_bank_with_AND(0x07, info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
	}
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_230(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m230.mode);

	return (EXIT_OK);
}
