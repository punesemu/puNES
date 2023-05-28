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
#include "cpu.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_180(void);

struct _m180 {
	BYTE reg;
} m180;

void map_init_180(void) {
	EXTCL_AFTER_MAPPER_INIT(180);
	EXTCL_CPU_WR_MEM(180);
	EXTCL_SAVE_MAPPER(180);
	mapper.internal_struct[0] = (BYTE *)&m180;
	mapper.internal_struct_size[0] = sizeof(m180);

	if (info.reset >= HARD) {
		memset(&m180, 0x00, sizeof(m180));
	}

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}
}
void extcl_after_mapper_init_180(void) {
	prg_fix_180();
}
void extcl_cpu_wr_mem_180(WORD address, BYTE value) {
	if (info.mapper.submapper == 2) {
		// bus conflict
		value &= prgrom_rd(address);
	}
	m180.reg = value;
	prg_fix_180();
}
BYTE extcl_save_mapper_180(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m180.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_180(void) {
	memmap_auto_16k(MMCPU(0x8000), 0);
	memmap_auto_16k(MMCPU(0xC000), m180.reg);
}
