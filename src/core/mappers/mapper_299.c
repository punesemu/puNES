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

INLINE static void prg_fix_299(void);
INLINE static void chr_fix_299(void);
INLINE static void mirroring_fix_299(void);

struct _m299 {
	BYTE reg;
} m299;

void map_init_299(void) {
	EXTCL_AFTER_MAPPER_INIT(299);
	EXTCL_CPU_WR_MEM(299);
	EXTCL_SAVE_MAPPER(299);
	map_internal_struct_init((BYTE *)&m299, sizeof(m299));

	memset(&m299, 0x00, sizeof(m299));
}
void extcl_after_mapper_init_299(void) {
	prg_fix_299();
	chr_fix_299();
	mirroring_fix_299();
}
void extcl_cpu_wr_mem_299(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m299.reg = value;
	prg_fix_299();
	chr_fix_299();
	mirroring_fix_299();
}
BYTE extcl_save_mapper_299(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m299.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_299(void) {
	memmap_auto_32k(0, MMCPU(0x8000), ((m299.reg & 0x70) >> 4));
}
INLINE static void chr_fix_299(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m299.reg & 0x70) >> 2) | (m299.reg & 0x03)));
}
INLINE static void mirroring_fix_299(void) {
	if (m299.reg & 0x80) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
