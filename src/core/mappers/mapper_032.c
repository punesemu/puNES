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

INLINE static void prg_fix_032(void);
INLINE static void chr_fix_032(void);
INLINE static void mirroring_fix_032(void);

struct m032 {
	BYTE prg[2];
	BYTE chr[8];
	BYTE reg;
} m032;

void map_init_032() {
	EXTCL_AFTER_MAPPER_INIT(032);
	EXTCL_CPU_WR_MEM(032);
	EXTCL_SAVE_MAPPER(032);
	mapper.internal_struct[0] = (BYTE *)&m032;
	mapper.internal_struct_size[0] = sizeof(m032);

	if (info.reset) {
		memset(&m032, 0x00, sizeof(m032));

		m032.prg[0] = 0;
		m032.prg[1] = 1;

		m032.chr[0] = 0;
		m032.chr[1] = 1;
		m032.chr[2] = 2;
		m032.chr[3] = 3;
		m032.chr[4] = 4;
		m032.chr[5] = 5;
		m032.chr[6] = 6;
		m032.chr[7] = 7;
	}
}
void extcl_after_mapper_init_032(void) {
	prg_fix_032();
	chr_fix_032();
	mirroring_fix_032();
}
void extcl_cpu_wr_mem_032(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0xA000:
			m032.prg[(address >> 13) & 0x01] = value;
			prg_fix_032();
			return;
		case 0x9000:
			m032.reg = value;
			prg_fix_032();
			mirroring_fix_032();
			return;
		case 0xB000:
			m032.chr[address & 0x07] = value;
			chr_fix_032();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_032(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m032.prg);
	save_slot_ele(mode, slot, m032.chr);
	save_slot_ele(mode, slot, m032.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_032(void) {
	WORD swap = (info.mapper.submapper == 1) ? 0 : (m032.reg & 0x02) << 13;

	memmap_auto_8k(MMCPU(0x8000 ^ swap), m032.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m032.prg[1]);
	memmap_auto_8k(MMCPU(0xC000 ^ swap), 0xFE);
	memmap_auto_8k(MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_032(void) {
	memmap_auto_1k(MMPPU(0x0000), m032.chr[0]);
	memmap_auto_1k(MMPPU(0x0400), m032.chr[1]);
	memmap_auto_1k(MMPPU(0x0800), m032.chr[2]);
	memmap_auto_1k(MMPPU(0x0C00), m032.chr[3]);
	memmap_auto_1k(MMPPU(0x1000), m032.chr[4]);
	memmap_auto_1k(MMPPU(0x1400), m032.chr[5]);
	memmap_auto_1k(MMPPU(0x1800), m032.chr[6]);
	memmap_auto_1k(MMPPU(0x1C00), m032.chr[7]);
}
INLINE static void mirroring_fix_032(void) {
	if (info.mapper.submapper == 1) {
		mirroring_SCR1();
	} else if (m032.reg & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
