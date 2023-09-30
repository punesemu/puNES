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

INLINE static void prg_fix_075(void);
INLINE static void chr_fix_075(void);
INLINE static void mirroring_fix_075(void);

struct m075 {
	BYTE prg[3];
	BYTE chr[2];
	BYTE other;
} m075;

void map_init_075(void) {
	EXTCL_AFTER_MAPPER_INIT(075);
	EXTCL_CPU_WR_MEM(075);
	EXTCL_SAVE_MAPPER(075);
	mapper.internal_struct[0] = (BYTE *)&m075;
	mapper.internal_struct_size[0] = sizeof(m075);

	if (info.reset >= HARD) {
		memset(&m075, 0x00, sizeof(m075));
	}
}
void extcl_after_mapper_init_075(void) {
	prg_fix_075();
	chr_fix_075();
	mirroring_fix_075();
}
void extcl_cpu_wr_mem_075(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0xA000:
		case 0xC000:
			m075.prg[(address >> 13) & 0x03] = value;
			prg_fix_075();
			return;
		case 0x9000:
			m075.other = value;
			chr_fix_075();
			mirroring_fix_075();
			return;
		case 0xE000:
		case 0xF000:
			m075.chr[(address >> 12) & 0x01] = value;
			chr_fix_075();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_075(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m075.prg);
	save_slot_ele(mode, slot, m075.chr);
	save_slot_ele(mode, slot, m075.other);

	return (EXIT_OK);
}

INLINE static void prg_fix_075(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m075.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m075.prg[1]);
	memmap_auto_8k(0, MMCPU(0xC000), m075.prg[2]);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_075(void) {
	memmap_auto_4k(0, MMPPU(0x0000), ((m075.other & 0x02) << 3) | (m075.chr[0] & 0x0F));
	memmap_auto_4k(0, MMPPU(0x1000), ((m075.other & 0x04) << 2) | (m075.chr[1] & 0x0F));
}
INLINE static void mirroring_fix_075(void) {
	if (info.mapper.mirroring == MIRRORING_FOURSCR) {
		mirroring_FSCR(0);
	} else if (m075.other & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}