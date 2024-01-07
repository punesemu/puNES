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

INLINE static void prg_fix_300(void);
INLINE static void chr_fix_300(void);
INLINE static void mirroring_fix_300(void);

struct _m300 {
	WORD reg;
} m300;

void map_init_300(void) {
	EXTCL_AFTER_MAPPER_INIT(300);
	EXTCL_CPU_WR_MEM(300);
	EXTCL_SAVE_MAPPER(300);
	map_internal_struct_init((BYTE *)&m300, sizeof(m300));

	if (info.reset >= HARD) {
		memset(&m300, 0x00, sizeof(m300));
	}
}
void extcl_after_mapper_init_300(void) {
	prg_fix_300();
	chr_fix_300();
	mirroring_fix_300();
}
void extcl_cpu_wr_mem_300(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m300.reg = address;
	prg_fix_300();
	chr_fix_300();
	mirroring_fix_300();
}
BYTE extcl_save_mapper_300(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m300.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_300(void) {
	WORD bank = (m300.reg & 0x1C) >> 2;

	memmap_auto_16k(0, MMCPU(0x8000), bank);
	memmap_auto_16k(0, MMCPU(0xC000), bank);
}
INLINE static void chr_fix_300(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m300.reg & 0x1C) >> 2));
}
INLINE static void mirroring_fix_300(void) {
	if (m300.reg & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
