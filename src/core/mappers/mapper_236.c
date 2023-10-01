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

INLINE static void prg_fix_236(void);
INLINE static void chr_fix_236(void);
INLINE static void mirroring_fix_236(void);

struct _m236 {
	WORD reg[3];
} m236;

void map_init_236(void) {
	EXTCL_AFTER_MAPPER_INIT(236);
	EXTCL_CPU_WR_MEM(236);
	EXTCL_CPU_RD_MEM(236);
	EXTCL_SAVE_MAPPER(236);
	mapper.internal_struct[0] = (BYTE *)&m236;
	mapper.internal_struct_size[0] = sizeof(m236);

	memset(&m236, 0x00, sizeof(m236));

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_236(void) {
	prg_fix_236();
	chr_fix_236();
	mirroring_fix_236();
}
void extcl_cpu_wr_mem_236(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m236.reg[(address >> 14) & 0x01] = address & 0xFF;
	prg_fix_236();
	chr_fix_236();
	mirroring_fix_236();
}
BYTE extcl_cpu_rd_mem_236(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		if (m236.reg[1] == 0x10) {
			address = (address & 0xFFF0) | dipswitch.value;
		}
		return (prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_236(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m236.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_236(void) {
	WORD bank = chrrom_size() ? (m236.reg[1] & 0x0F) : (m236.reg[1] & 0x07) | (m236.reg[0] << 3);

	switch(m236.reg[1] & 0x30) {
		case 0x00:
		case 0x10:
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), (bank | 0x07));
			return;
		case 0x20:
			memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
			return;
		case 0x30:
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), bank);
			return;
	}
}
INLINE static void chr_fix_236(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (chrrom_size() ? (m236.reg[0] &0x0F) : 0));
}
INLINE static void mirroring_fix_236(void) {
	if (m236.reg[0] & 0x20) {
		mirroring_H(0);
	} else  {
		mirroring_V(0);
	}
}
