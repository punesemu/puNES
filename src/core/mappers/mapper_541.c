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
#include "save_slot.h"

INLINE static void prg_fix_541(void);
INLINE static void mirroring_fix_541(void);

struct _m541 {
	WORD reg;
} m541;

void map_init_541(void) {
	EXTCL_AFTER_MAPPER_INIT(541);
	EXTCL_CPU_WR_MEM(541);
	EXTCL_SAVE_MAPPER(541);
	mapper.internal_struct[0] = (BYTE *)&m541;
	mapper.internal_struct_size[0] = sizeof(m541);

	memset(&m541, 0x00, sizeof(m541));
}
void extcl_after_mapper_init_541(void) {
	prg_fix_541();
	mirroring_fix_541();
}
void extcl_cpu_wr_mem_541(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	if (address >= 0xC000) {
		m541.reg = address;
		prg_fix_541();
		mirroring_fix_541();
	}
}
BYTE extcl_save_mapper_541(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m541.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_541(void) {
	if (m541.reg & 0x02) {
		WORD bank  = (m541.reg & 0xFF) >> 2;

		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), ((m541.reg & 0xFF) >> 3));
	}
}
INLINE static void mirroring_fix_541(void) {
	if (m541.reg & 0x01) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
