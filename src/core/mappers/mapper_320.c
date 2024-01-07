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

INLINE static void prg_fix_320(void);

struct _m320 {
	WORD reg[2];
} m320;

void map_init_320(void) {
	EXTCL_AFTER_MAPPER_INIT(320);
	EXTCL_CPU_WR_MEM(320);
	EXTCL_SAVE_MAPPER(320);
	map_internal_struct_init((BYTE *)&m320, sizeof(m320));

	memset(&m320, 0x00, sizeof(m320));
}
void extcl_after_mapper_init_320(void) {
	prg_fix_320();
}
void extcl_cpu_wr_mem_320(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address & 0xFFE0) == 0xF0E0) {
		m320.reg[0] = address;
	}
	m320.reg[1] = value;
	prg_fix_320();
}
BYTE extcl_save_mapper_320(BYTE mode, BYTE slot, UNUSED(FILE *fp)) {
	save_slot_ele(mode, slot, m320.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_320(void) {
	WORD base = m320.reg[0] << 3;
	WORD mask = 0x0F >> ((m320.reg[0] & 0x10) >> 4);
	WORD bank = base | (m320.reg[1] & mask);

	memmap_auto_16k(0, MMCPU(0x8000), bank);
	memmap_auto_16k(0, MMCPU(0xC000), (bank | (!(m320.reg[0] & 0x10) << 3) | 0x07));
}
