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

INLINE static void prg_fix_013(void);
INLINE static void chr_fix_013(void);

struct _m013 {
	BYTE reg;
} m013;

void map_init_013(void) {
	EXTCL_AFTER_MAPPER_INIT(013);
	EXTCL_CPU_WR_MEM(013);
	EXTCL_SAVE_MAPPER(013);
	map_internal_struct_init((BYTE *)&m013, sizeof(m013));

	if (info.reset >= HARD) {
		memset(&m013, 0x00, sizeof(m013));
	}
}
void extcl_after_mapper_init_013(void) {
	prg_fix_013();
	chr_fix_013();
}
void extcl_cpu_wr_mem_013(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	m013.reg = value & prgrom_rd(nidx, address);
	chr_fix_013();
}
BYTE extcl_save_mapper_013(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m013.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_013(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_013(void) {
	memmap_auto_4k(0, MMPPU(0x0000), 0);
	memmap_auto_4k(0, MMPPU(0x1000), m013.reg);
}
