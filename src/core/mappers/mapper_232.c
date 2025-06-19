/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_232(void);

struct _m232 {
	BYTE reg[2];
} m232;

void map_init_232(void) {
	EXTCL_AFTER_MAPPER_INIT(232);
	EXTCL_CPU_WR_MEM(232);
	EXTCL_SAVE_MAPPER(232);
	map_internal_struct_init((BYTE *)&m232, sizeof(m232));

	if (info.reset >= HARD) {
		memset(&m232, 0x00, sizeof(m232));
	}
}
void extcl_after_mapper_init_232(void) {
	prg_fix_232();
}
void extcl_cpu_wr_mem_232(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			if (info.mapper.submapper == 1) {
				m232.reg[0] = ((value & 0x10) >> 2) | (value & 0x08);
			} else {
				m232.reg[0] = (value & 0x18) >> 1;
			}
			prg_fix_232();
			return;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m232.reg[1] = value & 0x03;
			prg_fix_232();
			return;
	}
}
BYTE extcl_save_mapper_232(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m232.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_232(void) {
	memmap_auto_16k(0, MMCPU(0x8000), (m232.reg[0] | m232.reg[1]));
	memmap_auto_16k(0, MMCPU(0xC000), (m232.reg[0] | 0x03));
}
