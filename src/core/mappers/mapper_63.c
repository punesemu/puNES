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

struct _m63 {
	WORD reg;
} m63;

void map_init_63(void) {
	EXTCL_AFTER_MAPPER_INIT(63);
	EXTCL_CPU_WR_MEM(63);
	EXTCL_SAVE_MAPPER(63);
	EXTCL_WR_CHR(63);
	mapper.internal_struct[0] = (BYTE *)&m63;
	mapper.internal_struct_size[0] = sizeof(m63);

	if (info.reset >= HARD) {
		memset(&m63, 0x00, sizeof(m63));
	}
}
void extcl_after_mapper_init_63(void) {
	extcl_cpu_wr_mem_63(0x8000, 0x00);
}
void extcl_cpu_wr_mem_63(WORD address, BYTE value) {
	m63.reg = address;

	value = (m63.reg >> 2) & (info.mapper.submapper == 1 ? 0x7F : 0xFF);

	if (m63.reg & 0x0002) {
		value >>= 1;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	if (m63.reg & 0x0001) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
BYTE extcl_save_mapper_63(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m63.reg);

	return (EXIT_OK);
}
void extcl_wr_chr_63(WORD address, BYTE value) {
	if (!(info.mapper.submapper == 1 ? m63.reg & 0x0200 : m63.reg & 0x0400)) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
