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

INLINE static void prg_fix_285(void);
INLINE static void mirroring_fix_285(void);

struct _m285 {
	WORD reg;
} m285;

void map_init_285(void) {
	EXTCL_AFTER_MAPPER_INIT(285);
	EXTCL_CPU_WR_MEM(285);
	EXTCL_SAVE_MAPPER(285);
	mapper.internal_struct[0] = (BYTE *)&m285;
	mapper.internal_struct_size[0] = sizeof(m285);

	memset(&m285, 0x00, sizeof(m285));
}
void extcl_after_mapper_init_285(void) {
	prg_fix_285();
	mirroring_fix_285();
}
void extcl_cpu_wr_mem_285(UNUSED(WORD address), BYTE value) {
	m285.reg = value;
	prg_fix_285();
	mirroring_fix_285();
}
BYTE extcl_save_mapper_285(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m285.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_285(void) {
	if (m285.reg & 0x40) {
		memmap_auto_32k(MMCPU(0x8000), (m285.reg >> 1));
	} else {
		WORD bank = (m285.reg & 0x30) >> 1;

		memmap_auto_16k(MMCPU(0x8000), (bank | (m285.reg & 0x07)));
		memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
	}
}
INLINE static void mirroring_fix_285(void) {
	if (m285.reg & 0x80) {
		if (m285.reg & 0x20) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	} else if (m285.reg & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
