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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_306(void);
INLINE static void wram_fix_306(void);

struct _m306 {
	BYTE reg;
} m306;

void map_init_306(void) {
	EXTCL_AFTER_MAPPER_INIT(306);
	EXTCL_CPU_WR_MEM(306);
	EXTCL_SAVE_MAPPER(306);
	mapper.internal_struct[0] = (BYTE *)&m306;
	mapper.internal_struct_size[0] = sizeof(m306);

	if (info.reset >= HARD) {
		memset(&m306, 0x00, sizeof(m306));
	}
}
void extcl_after_mapper_init_306(void) {
	prg_fix_306();
	wram_fix_306();
}
void extcl_cpu_wr_mem_306(WORD address, UNUSED(BYTE value)) {
	if ((address & 0xD903) == 0xD903) {
		m306.reg = (address & 0x40) ? (address >> 2) & 0x0F : 0x08 | ((address >> 2) & 0x03);

		wram_fix_306();
	}
}
BYTE extcl_save_mapper_306(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m306.reg);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_306();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_306(void) {
	memmap_auto_32k(0x8000, 3);
}
INLINE static void wram_fix_306(void) {
	memmap_prgrom_8k(0x6000, m306.reg);
}
