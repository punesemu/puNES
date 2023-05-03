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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_093(void);

struct _m093 {
	BYTE reg;
} m093;

void map_init_093(void) {
	EXTCL_AFTER_MAPPER_INIT(093);
	EXTCL_CPU_WR_MEM(093);
	EXTCL_SAVE_MAPPER(093);
	EXTCL_RD_CHR(093);
	mapper.internal_struct[0] = (BYTE *)&m093;
	mapper.internal_struct_size[0] = sizeof(m093);

	if (info.reset >= HARD) {
		m093.reg = 0;
	}
}
void extcl_after_mapper_init_093(void) {
	prg_fix_093();
}
void extcl_cpu_wr_mem_093(UNUSED(WORD address), BYTE value) {
	// bus conflict
	m093.reg = value & prg_rom_rd(address);
	prg_fix_093();
}
BYTE extcl_save_mapper_093(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m093.reg);

	return (EXIT_OK);
}
BYTE extcl_rd_chr_093(WORD address) {
	BYTE openbus = address & 0xFF;

	return ((m093.reg & 0x01) ? chr.bank_1k[address >> 10][address & 0x3FF] : openbus);
}

INLINE static void prg_fix_093(void) {
	WORD bank = 0;

	bank = m093.reg >> 4;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = ~0;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
