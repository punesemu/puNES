/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_152(void);
INLINE static void chr_fix_152(void);
INLINE static void mirroring_fix_152(void);

struct _m152 {
	BYTE reg;
} m152;

void map_init_152(void) {
	EXTCL_AFTER_MAPPER_INIT(152);
	EXTCL_CPU_WR_MEM(152);
	EXTCL_SAVE_MAPPER(152);
	map_internal_struct_init((BYTE *)&m152, sizeof(m152));

	if (info.reset >= HARD) {
		m152.reg = 0;
	}
}
void extcl_after_mapper_init_152(void) {
	prg_fix_152();
	chr_fix_152();
	mirroring_fix_152();
}
void extcl_cpu_wr_mem_152(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	m152.reg = value & prgrom_rd(nidx, address);
	prg_fix_152();
	chr_fix_152();
	mirroring_fix_152();
}
BYTE extcl_save_mapper_152(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m152.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_152(void) {
	memmap_auto_16k(0, MMCPU(0x8000), (m152.reg >> 4));
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_152(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m152.reg & 0x0F));
}
INLINE static void mirroring_fix_152(void) {
	if (m152.reg & 0x80) {
		mirroring_SCR1(0);
	} else {
		mirroring_SCR0(0);
	}
}
