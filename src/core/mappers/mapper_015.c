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

INLINE static void prg_fix_015(void);
INLINE static void mirroring_fix_015(void);

struct _m015 {
	WORD reg[2];
} m015;

void map_init_015(void) {
	EXTCL_AFTER_MAPPER_INIT(015);
	EXTCL_CPU_WR_MEM(015);
	EXTCL_SAVE_MAPPER(015);
	mapper.internal_struct[0] = (BYTE *)&m015;
	mapper.internal_struct_size[0] = sizeof(m015);

	memset(&m015, 0x00, sizeof(m015));
}
void extcl_after_mapper_init_015(void) {
	prg_fix_015();
	mirroring_fix_015();
}
void extcl_cpu_wr_mem_015(WORD address, BYTE value) {
	m015.reg[0] = address;
	m015.reg[1] = value;
	prg_fix_015();
	mirroring_fix_015();
}
BYTE extcl_save_mapper_015(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m015.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_015(void) {
	WORD bank = m015.reg[1] & 0x3F;

	switch (m015.reg[0] & 0x0003) {
		case 0:
			memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
			return;
		case 1:
			memmap_auto_16k(MMCPU(0x8000), bank);
			memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
			return;
		case 2:
			bank = (bank << 1) | (m015.reg[1] >> 7);
			memmap_auto_8k(MMCPU(0x8000), bank);
			memmap_auto_8k(MMCPU(0xA000), bank);
			memmap_auto_8k(MMCPU(0xC000), bank);
			memmap_auto_8k(MMCPU(0xE000), bank);
			return;
		case 3:
			memmap_auto_16k(MMCPU(0x8000), bank);
			memmap_auto_16k(MMCPU(0xC000), bank);
			return;
	}
}
INLINE static void mirroring_fix_015(void) {
	if (m015.reg[1] & 0x40) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
