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

INLINE static void prg_fix_382(void);
INLINE static void mirroring_fix_382(void);

struct _m382 {
	WORD reg[2];
} m382;

void map_init_382(void) {
	EXTCL_AFTER_MAPPER_INIT(382);
	EXTCL_CPU_WR_MEM(382);
	EXTCL_SAVE_MAPPER(382);
	mapper.internal_struct[0] = (BYTE *)&m382;
	mapper.internal_struct_size[0] = sizeof(m382);

	memset(&m382, 0x00, sizeof(m382));
}
void extcl_after_mapper_init_382(void) {
	prg_fix_382();
	mirroring_fix_382();
}
void extcl_cpu_wr_mem_382(WORD address, BYTE value) {
	if (!(m382.reg[0] & 0x0020)) {
		m382.reg[0] = address;
	}
	// bus conflict
	m382.reg[1] = value & prg_rom_rd(address);
	prg_fix_382();
	mirroring_fix_382();
}
BYTE extcl_save_mapper_382(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m382.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_382(void) {
	WORD bank = 0;

	if (m382.reg[0] & 0x0008) {
		bank = ((m382.reg[0] & 0x0007) << 2) | (m382.reg[1] & 0x0003);
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		bank = ((m382.reg[0] & 0x0007) << 3) | (m382.reg[1] & 0x0007);
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = ((m382.reg[0] & 0x0007) << 3) | 0x0007;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_382(void) {
	if (m382.reg[0] & 0x0010) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
