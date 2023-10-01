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

INLINE static void prg_fix_231(void);
INLINE static void mirroring_fix_231(void);

struct _m231 {
	WORD reg;
} m231;

void map_init_231(void) {
	EXTCL_AFTER_MAPPER_INIT(231);
	EXTCL_CPU_WR_MEM(231);
	EXTCL_SAVE_MAPPER(231);
	mapper.internal_struct[0] = (BYTE *)&m231;
	mapper.internal_struct_size[0] = sizeof(m231);

	if (info.reset >= HARD) {
		memset(&m231, 0x00, sizeof(m231));
	}
}
void extcl_after_mapper_init_231(void) {
	prg_fix_231();
	mirroring_fix_231();
}
void extcl_cpu_wr_mem_231(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m231.reg = address;
	prg_fix_231();
	mirroring_fix_231();
}
BYTE extcl_save_mapper_231(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m231.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_231(void) {
	WORD bank = m231.reg & 0x1E;

	if (m231.reg & 0x20) {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	}
}
INLINE static void mirroring_fix_231(void) {
	switch ((m231.reg & 0xC0) >> 6) {
		case 0:
			mirroring_SCR0(0);
			break;
		case 1:
			mirroring_V(0);
			break;
		case 2:
			mirroring_H(0);
			break;
		case 3:
			mirroring_SCR0x1_SCR1x3(0);
			break;
	}
}
