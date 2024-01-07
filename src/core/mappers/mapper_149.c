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

INLINE static void prg_fix_149(void);
INLINE static void chr_fix_149(void);

struct _m149 {
	BYTE reg;
} m149;

void map_init_149(void) {
	EXTCL_AFTER_MAPPER_INIT(149);
	EXTCL_CPU_WR_MEM(149);
	EXTCL_SAVE_MAPPER(149);
	map_internal_struct_init((BYTE *)&m149, sizeof(m149));

	if (info.reset >= HARD) {
		memset(&m149, 0x00, sizeof(m149));
	}
}
void extcl_after_mapper_init_149(void) {
	prg_fix_149();
	chr_fix_149();
}
void extcl_cpu_wr_mem_149(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m149.reg = value;
	chr_fix_149();
}
BYTE extcl_save_mapper_149(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m149.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_149(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_149(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m149.reg & 0x80) >> 7));
}
