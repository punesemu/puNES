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

INLINE static void prg_fix_521(void);

struct _m521 {
	BYTE reg;
} m521;

void map_init_521(void) {
	EXTCL_AFTER_MAPPER_INIT(521);
	EXTCL_CPU_WR_MEM(521);
	EXTCL_SAVE_MAPPER(521);
	map_internal_struct_init((BYTE *)&m521, sizeof(m521));

	if (info.reset >= HARD) {
		memset(&m521, 0x00, sizeof(m521));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_521(void) {
	prg_fix_521();
}
void extcl_cpu_wr_mem_521(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m521.reg = value;
		prg_fix_521();
	}
}
BYTE extcl_save_mapper_521(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m521.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_521(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m521.reg);
	memmap_auto_16k(0, MMCPU(0xC000), 0x08);
}
