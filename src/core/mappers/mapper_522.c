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

INLINE static void prg_fix_522(void);
INLINE static void wram_fix_522(void);

struct _m522 {
	BYTE ind;
	BYTE reg[8];
} m522;

void map_init_522(void) {
	EXTCL_AFTER_MAPPER_INIT(522);
	EXTCL_CPU_WR_MEM(522);
	EXTCL_SAVE_MAPPER(522);
	mapper.internal_struct[0] = (BYTE *)&m522;
	mapper.internal_struct_size[0] = sizeof(m522);

	memset(&m522, 0x00, sizeof(m522));
}
void extcl_after_mapper_init_522(void) {
	prg_fix_522();
	wram_fix_522();
}
void extcl_cpu_wr_mem_522(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xE000:
		case 0xF000:
			m522.ind = value & 0x07;
			return;
		case 0x8001:
		case 0x9001:
		case 0xA001:
		case 0xB001:
		case 0xE001:
		case 0xF001:
			m522.reg[m522.ind] = value;
			prg_fix_522();
			wram_fix_522();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_522(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m522.ind);
	save_slot_ele(mode, slot, m522.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_522(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m522.reg[6]);
	memmap_auto_8k(0, MMCPU(0xA000), m522.reg[7]);
	memmap_wram_8k(0, MMCPU(0xC000), 0);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void wram_fix_522(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000), 0xFE);
}
