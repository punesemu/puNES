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

INLINE static void prg_fix_290(void);
INLINE static void chr_fix_290(void);
INLINE static void mirroring_fix_290(void);

struct _m290 {
	WORD reg;
} m290;

void map_init_290(void) {
	EXTCL_AFTER_MAPPER_INIT(290);
	EXTCL_CPU_WR_MEM(290);
	EXTCL_SAVE_MAPPER(290);
	map_internal_struct_init((BYTE *)&m290, sizeof(m290));

	memset(&m290, 0x00, sizeof(m290));
}
void extcl_after_mapper_init_290(void) {
	prg_fix_290();
	chr_fix_290();
	mirroring_fix_290();
}
void extcl_cpu_wr_mem_290(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m290.reg = address;
	prg_fix_290();
	chr_fix_290();
	mirroring_fix_290();
}
BYTE extcl_save_mapper_290(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m290.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_290(void) {
	WORD base = (m290.reg & 0x7800) >> 10;

	if (m290.reg & 0x80) {
		WORD bank = base | ((m290.reg & 0x40) >> 6);

		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), (base >> 1));
	}
}
INLINE static void chr_fix_290(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m290.reg & 0x0300) >> 5) | (m290.reg & 0x07));
}
INLINE static void mirroring_fix_290(void) {
	if (m290.reg & 0x400) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
