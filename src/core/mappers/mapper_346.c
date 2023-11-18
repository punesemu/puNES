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

#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_346(void);

struct _m346 {
	WORD reg;
} m346;

void map_init_346(void) {
	EXTCL_AFTER_MAPPER_INIT(346);
	EXTCL_CPU_WR_MEM(346);
	EXTCL_SAVE_MAPPER(346);
	mapper.internal_struct[0] = (BYTE *)&m346;
	mapper.internal_struct_size[0] = sizeof(m346);

	if (info.reset >= HARD) {
		m346.reg = 1;
	}
}
void extcl_after_mapper_init_346(void) {
	prg_fix_346();
}
void extcl_cpu_wr_mem_346(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	switch (address) {
		case 0xE0A0:
			m346.reg = 0;
			prg_fix_346();
			return;
		case 0xEE36:
			m346.reg = 1;
			prg_fix_346();
			return;
	}
}
BYTE extcl_save_mapper_346(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m346.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_346(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m346.reg);
}
