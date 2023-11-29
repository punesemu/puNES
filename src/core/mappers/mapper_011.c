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

INLINE static void prg_fix_011(void);
INLINE static void chr_fix_011(void);

struct _m011 {
	WORD reg;
} m011;

void map_init_011(void) {
	EXTCL_AFTER_MAPPER_INIT(011);
	EXTCL_CPU_WR_MEM(011);
	EXTCL_SAVE_MAPPER(011);
	map_internal_struct_init((BYTE *)&m011, sizeof(m011));

	if (info.reset >= HARD) {
		memset(&m011, 0x00, sizeof(m011));
	}
}
void extcl_after_mapper_init_011(void) {
	prg_fix_011();
	chr_fix_011();
}
void extcl_cpu_wr_mem_011(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m011.reg = value;
	prg_fix_011();
	chr_fix_011();
}
BYTE extcl_save_mapper_011(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m011.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_011(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m011.reg);
}
INLINE static void chr_fix_011(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m011.reg >> 4));
}
