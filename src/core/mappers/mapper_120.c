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

INLINE static void prg_fix_120(void);
INLINE static void wram_fix_120(void);

struct _m120 {
	BYTE reg;
} m120;

void map_init_120(void) {
	EXTCL_AFTER_MAPPER_INIT(120);
	EXTCL_CPU_WR_MEM(120);
	EXTCL_SAVE_MAPPER(120);
	mapper.internal_struct[0] = (BYTE *)&m120;
	mapper.internal_struct_size[0] = sizeof(m120);

	if (info.reset >= HARD) {
		memset(&m120, 0x00, sizeof(m120));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_120(void) {
	prg_fix_120();
	wram_fix_120();
}
void extcl_cpu_wr_mem_120(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x4000) && (address < 0x5FFF)) {
		if ((address & 0xFFF) == 0x01FF) {
			m120.reg = value;
			wram_fix_120();
		}
	}
}
BYTE extcl_save_mapper_120(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m120.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_120(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 2);
}
INLINE static void wram_fix_120(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000), m120.reg);
}
