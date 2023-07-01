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

INLINE static void prg_fix_101(void);
INLINE static void chr_fix_101(void);

struct _m101 {
	WORD reg;
} m101;

void map_init_101(void) {
	EXTCL_AFTER_MAPPER_INIT(101);
	EXTCL_CPU_WR_MEM(101);
	EXTCL_SAVE_MAPPER(101);
	mapper.internal_struct[0] = (BYTE *)&m101;
	mapper.internal_struct_size[0] = sizeof(m101);

	if (info.reset >= HARD) {
		memset(&m101, 0x00, sizeof(m101));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_101(void) {
	prg_fix_101();
	chr_fix_101();
}
void extcl_cpu_wr_mem_101(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m101.reg = value;
		prg_fix_101();
		chr_fix_101();
	}
}
BYTE extcl_save_mapper_101(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m101.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_101(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
INLINE static void chr_fix_101(void) {
	memmap_auto_8k(MMPPU(0x0000), m101.reg);
}
