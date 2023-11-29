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

INLINE static void prg_fix_112(void);
INLINE static void chr_fix_112(void);
INLINE static void mirroring_fix_112(void);

struct _m112 {
	BYTE index;
	BYTE reg[8];
	BYTE chr_outer;
	BYTE mirroring;
} m112;

void map_init_112(void) {
	EXTCL_AFTER_MAPPER_INIT(112);
	EXTCL_CPU_WR_MEM(112);
	EXTCL_SAVE_MAPPER(112);
	map_internal_struct_init((BYTE *)&m112, sizeof(m112));

	if (info.reset >= HARD) {
		memset(&m112, 0x00, sizeof(m112));

		m112.reg[0] = 0;
		m112.reg[1] = 1;
		m112.reg[2] = 0;
		m112.reg[3] = 2;
		m112.reg[4] = 4;
		m112.reg[5] = 5;
		m112.reg[6] = 6;
		m112.reg[7] = 7;
	}
}
void extcl_after_mapper_init_112(void) {
	prg_fix_112();
	chr_fix_112();
	mirroring_fix_112();
}
void extcl_cpu_wr_mem_112(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			m112.index = value;
			return;
		case 0xA000:
			m112.reg[m112.index] = value;
			prg_fix_112();
			chr_fix_112();
			return;
		case 0xC000:
			m112.chr_outer = value;
			chr_fix_112();
			return;
		case 0xE000:
			m112.mirroring = value;
			mirroring_fix_112();
			return;
	}
}
BYTE extcl_save_mapper_112(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m112.index);
	save_slot_ele(mode, slot, m112.reg);
	save_slot_ele(mode, slot, m112.chr_outer);
	save_slot_ele(mode, slot, m112.mirroring);
	return (EXIT_OK);
}

INLINE static void prg_fix_112(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m112.reg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m112.reg[1]);
	memmap_auto_8k(0, MMCPU(0xC000), 0xFE);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_112(void) {
	memmap_auto_2k(0, MMPPU(0x0000), (m112.reg[2] >> 1));
	memmap_auto_2k(0, MMPPU(0x0800), (m112.reg[3] >> 1));
	memmap_auto_1k(0, MMPPU(0x1000), (((m112.chr_outer & 0x10) << 4) | m112.reg[4]));
	memmap_auto_1k(0, MMPPU(0x1400), (((m112.chr_outer & 0x20) << 3) | m112.reg[5]));
	memmap_auto_1k(0, MMPPU(0x1800), (((m112.chr_outer & 0x40) << 2) | m112.reg[6]));
	memmap_auto_1k(0, MMPPU(0x1C00), (((m112.chr_outer & 0x80) << 1) | m112.reg[7]));
}
INLINE static void mirroring_fix_112(void) {
	if (m112.mirroring & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
