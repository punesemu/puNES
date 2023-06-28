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

INLINE static void prg_fix_007(void);
INLINE static void mirroring_fix_007(void);

struct _m007 {
	WORD reg;
} m007;

void map_init_007(void) {
	EXTCL_AFTER_MAPPER_INIT(007);
	EXTCL_CPU_WR_MEM(007);
	EXTCL_SAVE_MAPPER(007);
	mapper.internal_struct[0] = (BYTE *)&m007;
	mapper.internal_struct_size[0] = sizeof(m007);

	if (info.reset >= HARD) {
		memset(&m007, 0x00, sizeof(m007));
	}
}
void extcl_after_mapper_init_007(void) {
	prg_fix_007();
	mirroring_fix_007();
}
void extcl_cpu_wr_mem_007(WORD address, BYTE value) {
	// bus conflict
	if (info.mapper.submapper == 2) {
		value &= prgrom_rd(address);
	}
	m007.reg = value;
	prg_fix_007();
	mirroring_fix_007();
}
BYTE extcl_save_mapper_007(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m007.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_007(void) {
	memmap_auto_32k(MMCPU(0x8000), m007.reg);
}
INLINE static void mirroring_fix_007(void) {
	if (m007.reg & 0x10) {
		mirroring_SCR1();
	} else {
		mirroring_SCR0();
	}
}
