/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_564(void);
INLINE static void mirroring_fix_564(void);

struct _m564 {
	BYTE reg;
} m564;

void map_init_564(void) {
	EXTCL_AFTER_MAPPER_INIT(564);
	EXTCL_CPU_WR_MEM(564);
	EXTCL_SAVE_MAPPER(564);

	memset(&m564, 0x00, sizeof(m564));
}
void extcl_after_mapper_init_564(void) {
	prg_fix_564();
	mirroring_fix_564();
}
void extcl_cpu_wr_mem_564(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	if (m564.reg & 0x20) {
		if (m564.reg & 0x08) {
			m564.reg = (m564.reg & 0xE8) | (value & 0x17);
		} else {
			m564.reg = (m564.reg & 0xEC) | (value & 0x13);
		}
	} else {
		m564.reg = value;
	}
	prg_fix_564();
	mirroring_fix_564();
}
BYTE extcl_save_mapper_564(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m564.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_564(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m564.reg);
}
INLINE static void mirroring_fix_564(void) {
	switch (m564.reg & 0x30) {
		case 0x00:
			mirroring_V(0);
			return;
		case 0x10:
			mirroring_H(0);
			return;
		case 0x20:
			mirroring_SCR0(0);
			return;
		case 0x30:
			mirroring_SCR1(0);
			return;
	}
}
