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

INLINE static void prg_fix_002(void);

struct _m002 {
	BYTE reg;
} m002;

void map_init_002(void) {
	EXTCL_AFTER_MAPPER_INIT(002);
	EXTCL_CPU_WR_MEM(002);
	EXTCL_SAVE_MAPPER(002);
	mapper.internal_struct[0] = (BYTE *)&m002;
	mapper.internal_struct_size[0] = sizeof(m002);

	if (info.reset >= HARD) {
		memset(&m002, 0x00, sizeof(m002));
	}
}
void extcl_after_mapper_init_002(void) {
	prg_fix_002();
}
void extcl_cpu_wr_mem_002(BYTE nidx, WORD address, BYTE value) {
	if (info.mapper.submapper == 2) {
		// bus conflict
		value &= prgrom_rd(nidx, address);
	}
	m002.reg = value;
	prg_fix_002();
}
BYTE extcl_save_mapper_002(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m002.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_002(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m002.reg);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
