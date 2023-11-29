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

INLINE static void prg_fix_133(void);
INLINE static void chr_fix_133(void);

struct _m133 {
	BYTE reg;
} m133;

void map_init_133(void) {
	EXTCL_AFTER_MAPPER_INIT(133);
	EXTCL_CPU_WR_MEM(133);
	EXTCL_SAVE_MAPPER(133);
	map_internal_struct_init((BYTE *)&m133, sizeof(m133));

	if (info.reset >= HARD) {
		memset(&m133, 0x00, sizeof(m133));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_133(void) {
	prg_fix_133();
	chr_fix_133();
}
void extcl_cpu_wr_mem_133(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	if ((address >= 0x4000) && (address <= 0x5FFF) && (address & 0x0100)) {
		m133.reg = value;
		prg_fix_133();
		chr_fix_133();
	}
}
BYTE extcl_save_mapper_133(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m133.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_133(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m133.reg >> 2));
}
INLINE static void chr_fix_133(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m133.reg & 0x03));
}
