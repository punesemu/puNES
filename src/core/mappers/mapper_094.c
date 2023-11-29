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

INLINE static void prg_fix_094(void);

struct _m094 {
	BYTE reg;
} m094;

void map_init_094(void) {
	EXTCL_AFTER_MAPPER_INIT(094);
	EXTCL_CPU_WR_MEM(094);
	EXTCL_SAVE_MAPPER(094);
	map_internal_struct_init((BYTE *)&m094, sizeof(m094));

	if (info.reset >= HARD) {
		memset(&m094, 0x00, sizeof(m094));
	}
}
void extcl_after_mapper_init_094(void) {
	prg_fix_094();
}
void extcl_cpu_wr_mem_094(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	m094.reg = value & prgrom_rd(nidx, address);
	prg_fix_094();
}
BYTE extcl_save_mapper_094(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m094.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_094(void) {
	memmap_auto_16k(0, MMCPU(0x8000), (m094.reg >> 2));
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
