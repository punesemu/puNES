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

INLINE static void prg_fix_145(void);
INLINE static void chr_fix_145(void);

struct _m145 {
	BYTE reg;
} m145;

void map_init_145(void) {
	EXTCL_AFTER_MAPPER_INIT(145);
	EXTCL_CPU_WR_MEM(145);
	EXTCL_SAVE_MAPPER(145);
	map_internal_struct_init((BYTE *)&m145, sizeof(m145));

	if (info.reset >= HARD) {
		memset(&m145, 0x00, sizeof(m145));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_145(void) {
	prg_fix_145();
	chr_fix_145();
}
void extcl_cpu_wr_mem_145(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5000) && (address & 0x0100)) {
		m145.reg = value;
		chr_fix_145();
	}
}
BYTE extcl_save_mapper_145(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m145.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_145(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_145(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m145.reg >> 7));
}
