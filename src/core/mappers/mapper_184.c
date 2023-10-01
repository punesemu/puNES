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

INLINE static void prg_fix_184(void);
INLINE static void chr_fix_184(void);

struct _m184 {
	BYTE reg;
} m184;

void map_init_184(void) {
	EXTCL_AFTER_MAPPER_INIT(184);
	EXTCL_CPU_WR_MEM(184);
	EXTCL_SAVE_MAPPER(184);
	mapper.internal_struct[0] = (BYTE *)&m184;
	mapper.internal_struct_size[0] = sizeof(m184);

	if (info.reset >= HARD) {
		m184.reg = 0;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_184(void) {
	prg_fix_184();
	chr_fix_184();
}
void extcl_cpu_wr_mem_184(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m184.reg = value;
		prg_fix_184();
		chr_fix_184();
		return;
	}
}
BYTE extcl_save_mapper_184(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m184.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_184(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_184(void) {
	memmap_auto_4k(0, MMPPU(0x0000), (m184.reg & 0x0F));
	memmap_auto_4k(0, MMPPU(0x1000), (m184.reg >> 4));
}
