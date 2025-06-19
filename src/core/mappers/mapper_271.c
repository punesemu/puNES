/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_271(void);
INLINE static void chr_fix_271(void);
INLINE static void mirroring_fix_271(void);

struct _m271 {
	BYTE reg;
} m271;

void map_init_271(void) {
	EXTCL_AFTER_MAPPER_INIT(271);
	EXTCL_CPU_WR_MEM(271);
	EXTCL_SAVE_MAPPER(271);
	map_internal_struct_init((BYTE *)&m271, sizeof(m271));

	if (info.reset >= HARD) {
		memset(&m271, 0x00, sizeof(m271));
	}
}
void extcl_after_mapper_init_271(void) {
	prg_fix_271();
	chr_fix_271();
	mirroring_fix_271();
}
void extcl_cpu_wr_mem_271(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m271.reg = value;
	prg_fix_271();
	chr_fix_271();
	mirroring_fix_271();
}
BYTE extcl_save_mapper_271(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m271.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_271(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m271.reg >> 4));
}
INLINE static void chr_fix_271(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m271.reg & 0x0F));
}
INLINE static void mirroring_fix_271(void) {
	if (m271.reg & 0x20) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}