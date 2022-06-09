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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_396(void);
INLINE static void mirroring_fix_396(void);

struct _m396 {
	BYTE reg[2];
} m396;

void map_init_396(void) {
	EXTCL_AFTER_MAPPER_INIT(396);
	EXTCL_CPU_WR_MEM(396);
	EXTCL_SAVE_MAPPER(396);
	mapper.internal_struct[0] = (BYTE *)&m396;
	mapper.internal_struct_size[0] = sizeof(m396);

	if (info.reset >= HARD) {
		memset(&m396, 0x00, sizeof(m396));
	}
}
void extcl_after_mapper_init_396(void) {
	prg_fix_396();
	mirroring_fix_396();
}
void extcl_cpu_wr_mem_396(WORD address, BYTE value) {
	if ((address >= 0xA000) && (address <= 0xAFFF)) {
		m396.reg[1] = value;
	} else {
		m396.reg[0] = value;
	}
	prg_fix_396();
	mirroring_fix_396();
}
BYTE extcl_save_mapper_396(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m396.reg);

	return (EXIT_OK);
}
void extcl_wr_chr_396(WORD address, BYTE value) {
	if (m396.reg[2] & 0x20) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_396(void) {
	WORD base = (m396.reg[1] & 0x07) << 3;
	BYTE value;

	value = base | (m396.reg[0] & 0x07);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);

	value = base | 0x07;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_396(void) {
	if (m396.reg[1] & 0x60) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
