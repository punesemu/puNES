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

INLINE static void prg_fix_177(void);
INLINE static void mirroring_fix_177(void);

struct _m177 {
	BYTE reg;
} m177;

void map_init_177(void) {
	EXTCL_AFTER_MAPPER_INIT(177);
	EXTCL_CPU_WR_MEM(177);
	EXTCL_SAVE_MAPPER(177);
	mapper.internal_struct[0] = (BYTE *)&m177;
	mapper.internal_struct_size[0] = sizeof(m177);

	memset(&m177, 0x00, sizeof(m177));
}
void extcl_after_mapper_init_177(void) {
	prg_fix_177();
	mirroring_fix_177();
}
void extcl_cpu_wr_mem_177(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m177.reg = value;
	prg_fix_177();
	mirroring_fix_177();
}
BYTE extcl_save_mapper_177(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m177.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_177(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m177.reg & 0x1F));
}
INLINE static void mirroring_fix_177(void) {
	if (m177.reg & 0x20) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
