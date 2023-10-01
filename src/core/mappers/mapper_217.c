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

INLINE static void prg_fix_217(void);
INLINE static void chr_fix_217(void);

struct _m217 {
	WORD reg;
} m217;

void map_init_217(void) {
	EXTCL_AFTER_MAPPER_INIT(217);
	EXTCL_CPU_WR_MEM(217);
	EXTCL_SAVE_MAPPER(217);
	mapper.internal_struct[0] = (BYTE *)&m217;
	mapper.internal_struct_size[0] = sizeof(m217);

	memset(&m217, 0x00, sizeof(m217));
}
void extcl_after_mapper_init_217(void) {
	prg_fix_217();
	chr_fix_217();
}
void extcl_cpu_wr_mem_217(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m217.reg = address;
	prg_fix_217();
	chr_fix_217();
}
BYTE extcl_save_mapper_217(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m217.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_217(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m217.reg >> 2));
}
INLINE static void chr_fix_217(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m217.reg);
}
