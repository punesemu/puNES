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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_312(void);
INLINE static void mirroring_fix_312(void);

struct _m312 {
	WORD reg[2];
} m312;

void map_init_312(void) {
	EXTCL_AFTER_MAPPER_INIT(312);
	EXTCL_CPU_WR_MEM(312);
	EXTCL_SAVE_MAPPER(312);
	mapper.internal_struct[0] = (BYTE *)&m312;
	mapper.internal_struct_size[0] = sizeof(m312);

	if (info.reset >= HARD) {
		memset(&m312, 0x00, sizeof(m312));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_312(void) {
	prg_fix_312();
	mirroring_fix_312();
}
void extcl_cpu_wr_mem_312(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m312.reg[0] = value;
		prg_fix_312();
	} else if (address >= 0x8000) {
		m312.reg[1] = value;
		mirroring_fix_312();
	}
}
BYTE extcl_save_mapper_312(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m312.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_312(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m312.reg[0]);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void mirroring_fix_312(void) {
	if (m312.reg[1] & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
