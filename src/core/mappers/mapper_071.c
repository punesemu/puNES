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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_071(void);
INLINE static void mirroring_fix_071(void);

struct _m071 {
	BYTE prg;
	BYTE reg;
} m071;

void map_init_071(void) {
	EXTCL_AFTER_MAPPER_INIT(071);
	EXTCL_CPU_WR_MEM(071);
	EXTCL_SAVE_MAPPER(071);
	mapper.internal_struct[0] = (BYTE *)&m071;
	mapper.internal_struct_size[0] = sizeof(m071);

	if (info.reset >= HARD) {
		memset(&m071, 0x00, sizeof(m071));
	}

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}
}
void extcl_after_mapper_init_071(void) {
	prg_fix_071();
	mirroring_fix_071();
}
void extcl_cpu_wr_mem_071(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x9000:
			if (info.mapper.submapper == 1) {
				m071.reg = value;
				mirroring_fix_071();
			}
			return;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m071.prg = value;
			prg_fix_071();
			return;
	}
}
BYTE extcl_save_mapper_071(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m071.prg);
	save_slot_ele(mode, slot, m071.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_071(void) {
	memmap_auto_16k(MMCPU(0x8000), m071.prg);
	memmap_auto_16k(MMCPU(0xC000), 0xFF);
}
INLINE static void mirroring_fix_071(void) {
	if (info.mapper.submapper == 1) {
		if (m071.reg & 0x01) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	}
}
