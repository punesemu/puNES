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

#include "mappers.h"
#include "mem_map.h"
#include "info.h"
#include "save_slot.h"

INLINE static void prg_fix_125(void);
INLINE static void wram_fix_125(void);

struct _m125 {
	BYTE reg;
} m125;

void map_init_125(void) {
	EXTCL_AFTER_MAPPER_INIT(125);
	EXTCL_CPU_WR_MEM(125);
	EXTCL_CPU_RD_MEM(125);
	EXTCL_SAVE_MAPPER(125);
	mapper.internal_struct[0] = (BYTE *)&m125;
	mapper.internal_struct_size[0] = sizeof(m125);

	m125.reg = 0;

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_125(void) {
	prg_fix_125();
	wram_fix_125();
}
void extcl_cpu_wr_mem_125(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
			m125.reg = value;
			wram_fix_125();
			return;
		case 0xC000:
		case 0xD000:
			wram_byte(address & 0x1FFF) = value;
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_125(WORD address, BYTE openbus) {
	switch (address & 0xF000) {
		case 0xC000:
		case 0xD000:
			return (wram_byte(address & 0x1FFF));
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_125(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m125.reg);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_125();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_125(void) {
	WORD bank = 0;

	bank = 0xFC;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = 0xFD;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = 0x00;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void wram_fix_125(void) {
	memmap_prgrom_8k(0x6000, m125.reg);
}
