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
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_433(void);
INLINE static void mirroring_fix_433(void);

struct _m433 {
	BYTE reg;
} m433;

void map_init_433(void) {
	EXTCL_AFTER_MAPPER_INIT(433);
	EXTCL_CPU_WR_MEM(433);
	EXTCL_SAVE_MAPPER(433);
	EXTCL_WR_CHR(433);
	mapper.internal_struct[0] = (BYTE *)&m433;
	mapper.internal_struct_size[0] = sizeof(m433);

	memset(&m433, 0x00, sizeof(m433));
}
void extcl_after_mapper_init_433(void) {
	prg_fix_433();
	mirroring_fix_433();
}
void extcl_cpu_wr_mem_433(UNUSED(WORD address), BYTE value) {
	m433.reg = value;
	prg_fix_433();
	mirroring_fix_433();
}
BYTE extcl_save_mapper_433(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m433.reg);

	return (EXIT_OK);
}
void extcl_wr_chr_433(WORD address, BYTE value) {
	if (m433.reg & 0x80) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_433(void) {
	WORD bank = m433.reg & 0x1F;

	if (!(m433.reg & 0x20)) {
		bank >>= 1;
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_433(void) {
	if (m433.reg & 0x40) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
