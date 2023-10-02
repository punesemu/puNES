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
#include "save_slot.h"

INLINE static void prg_fix_343(void);
INLINE static void chr_fix_343(void);

struct _m343 {
	WORD reg;
} m343;

void map_init_343(void) {
	EXTCL_AFTER_MAPPER_INIT(343);
	EXTCL_CPU_WR_MEM(343);
	EXTCL_SAVE_MAPPER(343);
	mapper.internal_struct[0] = (BYTE *)&m343;
	mapper.internal_struct_size[0] = sizeof(m343);

	if (info.reset >= HARD) {
		m343.reg = 0xFF;
	}
}
void extcl_after_mapper_init_343(void) {
	prg_fix_343();
	chr_fix_343();
}
void extcl_cpu_wr_mem_343(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m343.reg = value;
}
BYTE extcl_save_mapper_343(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m343.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_343(void) {
	if (info.mapper.submapper == 1) {
		memmap_auto_32k(0, MMCPU(0x8000), m343.reg);
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), m343.reg);
		memmap_auto_16k(0, MMCPU(0xC000), m343.reg);
	}
}
INLINE static void chr_fix_343(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m343.reg);
}
