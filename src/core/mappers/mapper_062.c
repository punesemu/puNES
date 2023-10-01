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

INLINE static void prg_fix_062(void);
INLINE static void chr_fix_062(void);
INLINE static void mirroring_fix_062(void);

struct _m062 {
	WORD reg[2];
} m062;

void map_init_062(void) {
	EXTCL_AFTER_MAPPER_INIT(062);
	EXTCL_CPU_WR_MEM(062);
	EXTCL_SAVE_MAPPER(062);
	mapper.internal_struct[0] = (BYTE *)&m062;
	mapper.internal_struct_size[0] = sizeof(m062);

	memset(&m062, 0x00, sizeof(m062));
}
void extcl_after_mapper_init_062(void) {
	prg_fix_062();
	chr_fix_062();
	mirroring_fix_062();
}
void extcl_cpu_wr_mem_062(UNUSED(BYTE nidx), WORD address, BYTE value) {
	m062.reg[0] = address;
	m062.reg[1] = value;
	prg_fix_062();
	chr_fix_062();
	mirroring_fix_062();
}
BYTE extcl_save_mapper_062(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m062.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_062(void) {
	WORD bank = (m062.reg[0] & 0x40) | ((m062.reg[0] & 0x3F00) >> 8);

	if (m062.reg[0] & 0x20) {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	}
}
INLINE static void chr_fix_062(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m062.reg[0] & 0x1F) << 2) | (m062.reg[1] & 0x03)));
}
INLINE static void mirroring_fix_062(void) {
	if (m062.reg[0] & 0x80) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
