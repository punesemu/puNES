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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_190(void);
INLINE static void chr_fix_190(void);

struct _m190 {
	BYTE prg;
	WORD chr[4];
} m190;

void map_init_190(void) {
	EXTCL_AFTER_MAPPER_INIT(190);
	EXTCL_CPU_WR_MEM(190);
	EXTCL_SAVE_MAPPER(190);
	map_internal_struct_init((BYTE *)&m190, sizeof(m190));

	if (info.reset >= HARD) {
		memset(&m190, 0x00, sizeof(m190));
	}
}
void extcl_after_mapper_init_190(void) {
	prg_fix_190();
	chr_fix_190();
}
void extcl_cpu_wr_mem_190(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			m190.prg = value & 0x07;
			prg_fix_190();
			return;
		case 0xA000:
			m190.chr[address & 0x03] = value;
			chr_fix_190();
			return;
		case 0xC000:
			m190.prg = 0x08 | (value & 0x07);
			prg_fix_190();
			return;
	}
}
BYTE extcl_save_mapper_190(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m190.prg);
	save_slot_ele(mode, slot, m190.chr);
	return (EXIT_OK);
}

INLINE static void prg_fix_190(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m190.prg);
	memmap_auto_16k(0, MMCPU(0xC000), 0);
}
INLINE static void chr_fix_190(void) {
	memmap_auto_2k(0, MMPPU(0x0000), m190.chr[0]);
	memmap_auto_2k(0, MMPPU(0x0800), m190.chr[1]);
	memmap_auto_2k(0, MMPPU(0x1000), m190.chr[2]);
	memmap_auto_2k(0, MMPPU(0x1800), m190.chr[3]);
}
