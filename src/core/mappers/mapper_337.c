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

INLINE static void prg_fix_337(void);
INLINE static void wram_fix_337(void);
INLINE static void mirroring_fix_337(void);

struct _m337 {
	BYTE reg;
} m337;

void map_init_337(void) {
	EXTCL_AFTER_MAPPER_INIT(337);
	EXTCL_CPU_WR_MEM(337);
	EXTCL_SAVE_MAPPER(337);
	mapper.internal_struct[0] = (BYTE *)&m337;
	mapper.internal_struct_size[0] = sizeof(m337);

	if (info.reset >= HARD) {
		memset(&m337, 0x00, sizeof(m337));
	}
}
void extcl_after_mapper_init_337(void) {
	prg_fix_337();
	wram_fix_337();
	mirroring_fix_337();
}
void extcl_cpu_wr_mem_337(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			m337.reg = (m337.reg & 0x07) | (value & ~0x07);
			prg_fix_337();
			mirroring_fix_337();
			return;
		case 0xC000:
		case 0xE000:
			m337.reg = (m337.reg & ~0x07) | (value & 0x07);
			prg_fix_337();
			mirroring_fix_337();
			return;
	}
}
BYTE extcl_save_mapper_337(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m337.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_337(void) {
	switch ((m337.reg >> 6) & 0x03) {
		case 0:
			memmap_auto_16k(MMCPU(0x8000), m337.reg);
			memmap_auto_16k(MMCPU(0xC000), m337.reg);
			return;
		case 1:
			memmap_auto_32k(MMCPU(0x8000), (m337.reg >> 1));
			return;
		case 2:
		case 3:
			memmap_auto_16k(MMCPU(0x8000), m337.reg);
			memmap_auto_16k(MMCPU(0xC000), (m337.reg | 0x07));
			return;
	}
}
INLINE static void wram_fix_337(void) {
	memmap_prgrom_8k(MMCPU(0x6000), 1);
}
INLINE static void mirroring_fix_337(void) {
	if (m337.reg & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
