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

INLINE static void prg_fix_329(void);
INLINE static void wram_fix_329(void);
INLINE static void mirroring_fix_329(void);

struct _m329 {
	BYTE reg;
} m329;

void map_init_329(void) {
	EXTCL_AFTER_MAPPER_INIT(329);
	EXTCL_CPU_WR_MEM(329);
	EXTCL_SAVE_MAPPER(329);
	mapper.internal_struct[0] = (BYTE *)&m329;
	mapper.internal_struct_size[0] = sizeof(m329);

	memset(&m329, 0x00, sizeof(m329));

//	if (wram_size() < 0x8000) {
//		wram_set_ram_size(0x8000);
//	}
}
void extcl_after_mapper_init_329(void) {
	prg_fix_329();
	wram_fix_329();
	mirroring_fix_329();
}
void extcl_cpu_wr_mem_329(UNUSED(WORD address), BYTE value) {
	m329.reg = value;
	prg_fix_329();
	wram_fix_329();
	mirroring_fix_329();
}
BYTE extcl_save_mapper_329(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m329.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_329(void) {
	WORD bank = m329.reg & 0x1F;

	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);

	map_prg_rom_8k_update();
}
INLINE static void wram_fix_329(void) {
	memmap_auto_8k(MMCPU(0x6000), m329.reg >> 6);
}
INLINE static void mirroring_fix_329(void) {
	if (m329.reg & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
