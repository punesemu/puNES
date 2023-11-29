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

INLINE static void prg_fix_063(void);
INLINE static void chr_fix_063(void);
INLINE static void mirroring_fix_063(void);

struct _m063 {
	WORD reg;
} m063;

void map_init_063(void) {
	EXTCL_AFTER_MAPPER_INIT(063);
	EXTCL_CPU_WR_MEM(063);
	EXTCL_SAVE_MAPPER(063);
	map_internal_struct_init((BYTE *)&m063, sizeof(m063));

	memset(&m063, 0x00, sizeof(m063));
}
void extcl_after_mapper_init_063(void) {
	prg_fix_063();
	chr_fix_063();
	mirroring_fix_063();
}
void extcl_cpu_wr_mem_063(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m063.reg = address;
	prg_fix_063();
	chr_fix_063();
	mirroring_fix_063();
}
BYTE extcl_save_mapper_063(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m063.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_063(void) {
	WORD bank = (m063.reg >> 2) & (info.mapper.submapper == 1 ? 0x7F : 0xFF);

	if (m063.reg & 0x0002) {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_063(void) {
	BYTE enabled = !(((info.mapper.submapper == 0) && (m063.reg & 0x0400)) ||
		((info.mapper.submapper == 1) && (m063.reg & 0x0200)));

	memmap_auto_wp_8k(0, MMPPU(0x0000), 0, TRUE, enabled);
}
INLINE static void mirroring_fix_063(void) {
	if (m063.reg & 0x0001) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
