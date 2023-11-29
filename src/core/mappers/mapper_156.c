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

INLINE static void prg_fix_156(void);
INLINE static void chr_fix_156(void);
INLINE static void mirroring_fix_156(void);

struct _m156 {
	BYTE prg;
	WORD chr[8];
	BYTE mirroring;
} m156;

void map_init_156(void) {
	EXTCL_AFTER_MAPPER_INIT(156);
	EXTCL_CPU_WR_MEM(156);
	EXTCL_SAVE_MAPPER(156);
	map_internal_struct_init((BYTE *)&m156, sizeof(m156));

	if (info.reset >= HARD) {
		memset(&m156, 0x00, sizeof(m156));

		m156.mirroring = 2;
	}
}
void extcl_after_mapper_init_156(void) {
	prg_fix_156();
	chr_fix_156();
	mirroring_fix_156();
}
void extcl_cpu_wr_mem_156(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	switch (address & 0xCFFC) {
		case 0xC000:
			m156.chr[address & 0x03] = (m156.chr[address & 0x03] & 0xFF00) | value;
			chr_fix_156();
			return;
		case 0xC004:
			m156.chr[address & 0x03] = (m156.chr[address & 0x03] & 0x00FF) | (value << 8);
			chr_fix_156();
			return;
		case 0xC008:
			m156.chr[0x04 | (address & 0x03)] = (m156.chr[0x04 | (address & 0x03)] & 0xFF00) | value;
			chr_fix_156();
			return;
		case 0xC00C:
			m156.chr[0x04 | (address & 0x03)] = (m156.chr[0x04 | (address & 0x03)] & 0x00FF) | (value << 8);
			chr_fix_156();
			return;
		case 0xC010:
			m156.prg = value;
			prg_fix_156();
			return;
		case 0xC014:
			m156.mirroring = value & 0x01;
			mirroring_fix_156();
			return;
	}
}
BYTE extcl_save_mapper_156(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m156.prg);
	save_slot_ele(mode, slot, m156.chr);
	save_slot_ele(mode, slot, m156.mirroring);
	return (EXIT_OK);
}

INLINE static void prg_fix_156(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m156.prg);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_156(void) {
	memmap_auto_1k(0, MMPPU(0x0000), m156.chr[0]);
	memmap_auto_1k(0, MMPPU(0x0400), m156.chr[1]);
	memmap_auto_1k(0, MMPPU(0x0800), m156.chr[2]);
	memmap_auto_1k(0, MMPPU(0x0C00), m156.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1000), m156.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1400), m156.chr[5]);
	memmap_auto_1k(0, MMPPU(0x1800), m156.chr[6]);
	memmap_auto_1k(0, MMPPU(0x1C00), m156.chr[7]);
}
INLINE static void mirroring_fix_156(void) {
	switch (m156.mirroring) {
		case 0:
			mirroring_V(0);
			return;
		case 1:
			mirroring_H(0);
			return;
		default:
			mirroring_SCR0(0);
			return;
	}
}
