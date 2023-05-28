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

INLINE static void prg_fix_003(void);
INLINE static void chr_fix_003(void);
INLINE static void wram_fix_003(void);

struct m003 {
	BYTE reg;
} m003;

void map_init_003() {
	EXTCL_AFTER_MAPPER_INIT(003);
	EXTCL_CPU_WR_MEM(003);
	EXTCL_SAVE_MAPPER(003);
	mapper.internal_struct[0] = (BYTE *)&m003;
	mapper.internal_struct_size[0] = sizeof(m003);

	if (info.reset) {
		memset(&m003, 0x00, sizeof(m003));
	}

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}
}
void extcl_after_mapper_init_003(void) {
	prg_fix_003();
	chr_fix_003();
	wram_fix_003();
}
void extcl_cpu_wr_mem_003(WORD address, BYTE value) {
	// bus conflict
	if (info.mapper.submapper == 2) {
		value &= prgrom_rd(address);
	}
	m003.reg = value;
	chr_fix_003();
}
BYTE extcl_save_mapper_003(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m003.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_003(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
INLINE static void chr_fix_003(void) {
	memmap_auto_8k(MMPPU(0x0000), m003.reg);
}
INLINE static void wram_fix_003(void) {
	memmap_wram_8k(MMCPU(0x6000), 0);
}

