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

INLINE static void prg_fix_340(void);
INLINE static void chr_fix_340(void);
INLINE static void mirroring_fix_340(void);

struct _m340 {
	WORD reg;
} m340;

void map_init_340(void) {
	EXTCL_AFTER_MAPPER_INIT(340);
	EXTCL_CPU_WR_MEM(340);
	EXTCL_SAVE_MAPPER(340);
	map_internal_struct_init((BYTE *)&m340, sizeof(m340));

	memset(&m340, 0x00, sizeof(m340));
}
void extcl_after_mapper_init_340(void) {
	prg_fix_340();
	chr_fix_340();
	mirroring_fix_340();
}
void extcl_cpu_wr_mem_340(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m340.reg = address;
	prg_fix_340();
	chr_fix_340();
	mirroring_fix_340();
}
BYTE extcl_save_mapper_340(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m340.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_340(void) {
	WORD bank = ((m340.reg & 0x80) >> 2) | (m340.reg & 0x1F);

	if (m340.reg & 0x20) {
		if (!(m340.reg & 0x01)) {
			memmap_auto_32k(0, MMCPU(0x8000), (((m340.reg & 0x80) >> 3) | ((m340.reg & 0x1E) >> 1)));
		} else {
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), bank);
		}
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), (bank | 0x07));
	}
}
INLINE static void chr_fix_340(void) {
	memmap_vram_wp_8k(0, MMPPU(0x0000), 0, TRUE, !(m340.reg & 0x20));
}
INLINE static void mirroring_fix_340(void) {
	if (((m340.reg & 0x25) == 0x25) || (m340.reg & 0x40)) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
