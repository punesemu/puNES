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

INLINE static void prg_fix_525(void);
INLINE static void chr_fix_525(void);
INLINE static void mirroring_fix_525(void);

struct _m525 {
	BYTE prg;
	BYTE chr[8];
	BYTE mirroring;
} m525;

void map_init_525(void) {
	EXTCL_AFTER_MAPPER_INIT(525);
	EXTCL_CPU_WR_MEM(525);
	EXTCL_SAVE_MAPPER(525);
	map_internal_struct_init((BYTE *)&m525, sizeof(m525));

	if (info.reset >= HARD) {
		memset(&m525, 0x00, sizeof(m525));
	}
}
void extcl_after_mapper_init_525(void) {
	prg_fix_525();
	chr_fix_525();
	mirroring_fix_525();
}
void extcl_cpu_wr_mem_525(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m525.prg = value;
			prg_fix_525();
			return;
		case 0x9000:
			m525.mirroring = value;
			mirroring_fix_525();
			return;
		case 0xB000:
			m525.chr[address & 0x07] = value;
			chr_fix_525();
			return;
	}
}
BYTE extcl_save_mapper_525(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m525.prg);
	save_slot_ele(mode, slot, m525.chr);
	save_slot_ele(mode, slot, m525.mirroring);
	return (EXIT_OK);
}

INLINE static void prg_fix_525(void) {
	memmap_auto_16k(0, MMCPU(0x8000), (m525.prg >> 1));
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_525(void) {
	memmap_auto_1k(0, MMPPU(0x0000), m525.chr[0]);
	memmap_auto_1k(0, MMPPU(0x0400), m525.chr[1]);
	memmap_auto_1k(0, MMPPU(0x0800), m525.chr[2]);
	memmap_auto_1k(0, MMPPU(0x0C00), m525.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1000), m525.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1400), m525.chr[5]);
	memmap_auto_1k(0, MMPPU(0x1800), m525.chr[6]);
	memmap_auto_1k(0, MMPPU(0x1C00), m525.chr[7]);
}
INLINE static void mirroring_fix_525(void) {
	if (m525.mirroring & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
