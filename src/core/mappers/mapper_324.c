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

INLINE static void prg_fix_324(void);

struct _m324 {
	BYTE reg;
} m324;

void map_init_324(void) {
	EXTCL_AFTER_MAPPER_INIT(324);
	EXTCL_CPU_WR_MEM(324);
	EXTCL_SAVE_MAPPER(324);
	mapper.internal_struct[0] = (BYTE *)&m324;
	mapper.internal_struct_size[0] = sizeof(m324);

	memset(&m324, 0x00, sizeof(m324));
}
void extcl_after_mapper_init_324(void) {
	prg_fix_324();
}
void extcl_cpu_wr_mem_324(WORD address, UNUSED(BYTE value)) {
	if ((m324.reg & 0x08) || (m324.reg & 0x80) || !(value & 0x80)) {
		m324.reg = (m324.reg & 0xF8) | (value & 0x07);
	} else {
		// bus conflict
		m324.reg = value & prgrom_rd(address);
	}
	prg_fix_324();
}
BYTE extcl_save_mapper_324(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m324.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_324(void) {
	WORD bank = ((m324.reg & 0x70) >> 1) | (m324.reg & 0x07);

	memmap_auto_16k(MMCPU(0x8000), bank);
	memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
}
