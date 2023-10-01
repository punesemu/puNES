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

INLINE static void prg_fix_193(void);
INLINE static void chr_fix_193(void);
INLINE static void mirroring_fix_193(void);

struct _m193 {
	BYTE reg[8];
} m193;

void map_init_193(void) {
	EXTCL_AFTER_MAPPER_INIT(193);
	EXTCL_CPU_WR_MEM(193);
	EXTCL_SAVE_MAPPER(193);
	mapper.internal_struct[0] = (BYTE *)&m193;
	mapper.internal_struct_size[0] = sizeof(m193);

	if (info.reset >= HARD) {
		memset(&m193, 0x00, sizeof(m193));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_193(void) {
	prg_fix_193();
	chr_fix_193();
	mirroring_fix_193();
}
void extcl_cpu_wr_mem_193(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m193.reg[address & 0x07] = value;
		prg_fix_193();
		chr_fix_193();
		mirroring_fix_193();
	}
}
BYTE extcl_save_mapper_193(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m193.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_193(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m193.reg[3]);
	memmap_auto_8k(0, MMCPU(0xA000), 0xFD);
	memmap_auto_8k(0, MMCPU(0xC000), 0xFE);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_193(void) {
	memmap_auto_4k(0, MMPPU(0x0000), (m193.reg[0] >> 2));
	memmap_auto_2k(0, MMPPU(0x1000), (m193.reg[1] >> 1));
	memmap_auto_2k(0, MMPPU(0x1800), (m193.reg[2] >> 1));
}
INLINE static void mirroring_fix_193(void) {
	if (m193.reg[4] & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
