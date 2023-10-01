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

INLINE static void prg_fix_089(void);
INLINE static void chr_fix_089(void);
INLINE static void mirroring_fix_089(void);

struct _m089 {
	BYTE reg;
} m089;

void map_init_089(void) {
	EXTCL_AFTER_MAPPER_INIT(089);
	EXTCL_CPU_WR_MEM(089);
	EXTCL_SAVE_MAPPER(089);
	mapper.internal_struct[0] = (BYTE *)&m089;
	mapper.internal_struct_size[0] = sizeof(m089);

	if (info.reset >= HARD) {
		m089.reg = 0;
	}
}
void extcl_after_mapper_init_089(void) {
	prg_fix_089();
	chr_fix_089();
	mirroring_fix_089();
}
void extcl_cpu_wr_mem_089(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	m089.reg = value & prgrom_rd(nidx, address);
	prg_fix_089();
	chr_fix_089();
	mirroring_fix_089();
}
BYTE extcl_save_mapper_089(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m089.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_089(void) {
	memmap_auto_16k(0, MMCPU(0x8000), ((m089.reg >> 4) & 0x07));
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_089(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m089.reg & 0x80) >> 4) | (m089.reg & 0x07)));
}
INLINE static void mirroring_fix_089(void) {
	if (m089.reg & 0x08) {
		mirroring_SCR1(0);
	} else {
		mirroring_SCR0(0);
	}
}
