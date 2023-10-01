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

INLINE static void prg_fix_066(void);
INLINE static void chr_fix_066(void);

struct _m066 {
	BYTE reg;
} m066;

void map_init_066(void) {
	EXTCL_AFTER_MAPPER_INIT(066);
	EXTCL_CPU_WR_MEM(066);
	EXTCL_SAVE_MAPPER(066);
	mapper.internal_struct[0] = (BYTE *)&m066;
	mapper.internal_struct_size[0] = sizeof(m066);

	if (info.reset >= HARD) {
		memset(&m066, 0x00, sizeof(m066));
	}
}
void extcl_after_mapper_init_066(void) {
	prg_fix_066();
	chr_fix_066();
}
void extcl_cpu_wr_mem_066(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m066.reg = value;
	prg_fix_066();
	chr_fix_066();
}
BYTE extcl_save_mapper_066(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m066.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_066(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m066.reg >> 4));
}
INLINE static void chr_fix_066(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m066.reg & 0x0F));
}
