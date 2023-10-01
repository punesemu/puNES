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

INLINE static void prg_fix_429(void);
INLINE static void chr_fix_429(void);

struct _m429 {
	BYTE reg;
} m429;

void map_init_429(void) {
	EXTCL_AFTER_MAPPER_INIT(429);
	EXTCL_CPU_WR_MEM(429);
	EXTCL_SAVE_MAPPER(429);
	mapper.internal_struct[0] = (BYTE *)&m429;
	mapper.internal_struct_size[0] = sizeof(m429);

	m429.reg = 0x04;
}
void extcl_after_mapper_init_429(void) {
	prg_fix_429();
	chr_fix_429();
}
void extcl_cpu_wr_mem_429(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m429.reg = value;
	prg_fix_429();
	chr_fix_429();
}
BYTE extcl_save_mapper_429(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m429.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_429(void) {
	memmap_auto_32k(0, MMCPU(0x8000), ((m429.reg & 0x3C) >> 2));
}
INLINE static void chr_fix_429(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m429.reg & 0x03));
}
