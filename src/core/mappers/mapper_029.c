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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_029(void);
INLINE static void chr_fix_029(void);

struct _m029 {
	BYTE reg;
} m029;

void map_init_029(void) {
	EXTCL_AFTER_MAPPER_INIT(029);
	EXTCL_CPU_WR_MEM(029);
	EXTCL_SAVE_MAPPER(029);
	mapper.internal_struct[0] = (BYTE *)&m029;
	mapper.internal_struct_size[0] = sizeof(m029);

	memset(&m029, 0x00, sizeof(m029));
}
void extcl_after_mapper_init_029(void) {
	prg_fix_029();
	chr_fix_029();
}
void extcl_cpu_wr_mem_029(UNUSED(WORD address), BYTE value) {
	m029.reg = value;
	prg_fix_029();
	chr_fix_029();
}
BYTE extcl_save_mapper_029(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m029.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_029(void) {
	WORD bank = 0;

	bank = m029.reg >> 2;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_029(void) {
	map_chr_rom_8k(m029.reg & 0x03);
}
