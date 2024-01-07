/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_041(void);
INLINE static void chr_fix_041(void);
INLINE static void mirroring_fix_041(void);

struct _m041 {
	WORD reg[2];
} m041;

void map_init_041(void) {
	EXTCL_AFTER_MAPPER_INIT(041);
	EXTCL_CPU_WR_MEM(041);
	EXTCL_SAVE_MAPPER(041);
	map_internal_struct_init((BYTE *)&m041, sizeof(m041));

	memset(&m041, 0x00, sizeof(m041));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_041(void) {
	prg_fix_041();
	chr_fix_041();
	mirroring_fix_041();
}
void extcl_cpu_wr_mem_041(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		if (!(address & 0x0800)) {
			m041.reg[0] = address;
			prg_fix_041();
			chr_fix_041();
			mirroring_fix_041();
		}
	} else if (address >= 0x8000) {
		if (m041.reg[0] & 0x04) {
			// bus conflict
			m041.reg[1] = value & prgrom_rd(nidx, address);
			chr_fix_041();
		}
	}
}
BYTE extcl_save_mapper_041(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m041.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_041(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m041.reg[0] & 0x07));
}
INLINE static void chr_fix_041(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m041.reg[0] & 0x18) >> 1) | (m041.reg[1] & 0x03)));
}
INLINE static void mirroring_fix_041(void) {
	if (m041.reg[0] & 0x20) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
