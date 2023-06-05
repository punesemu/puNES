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

INLINE static void prg_fix_409(void);

struct _m409 {
	WORD reg;
} m409;

void map_init_409(void) {
	EXTCL_AFTER_MAPPER_INIT(409);
	EXTCL_CPU_WR_MEM(409);
	EXTCL_SAVE_MAPPER(409);
	mapper.internal_struct[0] = (BYTE *)&m409;
	mapper.internal_struct_size[0] = sizeof(m409);

	if (info.reset >= HARD) {
		memset(&m409, 0x00, sizeof(m409));
	}
}
void extcl_after_mapper_init_409(void) {
	prg_fix_409();
}
void extcl_cpu_wr_mem_409(WORD address, UNUSED(BYTE value)) {
	if ((address >= 0xC000) && (address <= 0xCFFF)) {
		m409.reg = address;
		prg_fix_409();
	}
}
BYTE extcl_save_mapper_409(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m409.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_409(void) {
	memmap_auto_16k(MMCPU(0x8000), m409.reg);
	memmap_auto_16k(MMCPU(0xC000), 0xFFFF);
}
