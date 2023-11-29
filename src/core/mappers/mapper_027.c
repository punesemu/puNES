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

INLINE static void prg_fix_027(void);
INLINE static void chr_fix_027(void);
INLINE static void mirroring_fix_027(void);

struct _m027 {
	WORD reg;
} m027;

void map_init_027(void) {
	EXTCL_AFTER_MAPPER_INIT(027);
	EXTCL_CPU_WR_MEM(027);
	EXTCL_SAVE_MAPPER(027);
	map_internal_struct_init((BYTE *)&m027, sizeof(m027));

	if (info.reset >= HARD) {
		memset(&m027, 0x00, sizeof(m027));
	}
}
void extcl_after_mapper_init_027(void) {
	prg_fix_027();
	chr_fix_027();
	mirroring_fix_027();
}
void extcl_cpu_wr_mem_027(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m027.reg = address;
	prg_fix_027();
	chr_fix_027();
	mirroring_fix_027();
}
BYTE extcl_save_mapper_027(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m027.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_027(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_027(void) {
	WORD bank = ((m027.reg & 0x01) << 1) | (m027.reg & 0x01);

	memmap_auto_4k(0, MMPPU(0x0000), bank);
	memmap_auto_4k(0, MMPPU(0x1000), bank);
}
INLINE static void mirroring_fix_027(void) {
	if (m027.reg & 0x01) {
		mirroring_SCR1(0);
	} else {
		mirroring_SCR0(0);
	}
}
