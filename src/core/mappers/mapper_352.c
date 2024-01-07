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

INLINE static void prg_fix_352(void);
INLINE static void chr_fix_352(void);
INLINE static void mirroring_fix_352(void);

struct _m352 {
	BYTE chip;
} m352;

void map_init_352(void) {
	EXTCL_AFTER_MAPPER_INIT(352);
	EXTCL_CPU_WR_MEM(352);
	EXTCL_SAVE_MAPPER(352);
	map_internal_struct_init((BYTE *)&m352, sizeof(m352));

	if (info.reset >= HARD) {
		m352.chip = 0;
	} else {
		m352.chip++;
	}
}
void extcl_after_mapper_init_352(void) {
	prg_fix_352();
	chr_fix_352();
	mirroring_fix_352();
}
void extcl_cpu_wr_mem_352(UNUSED(BYTE nidx), UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_save_mapper_352(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m352.chip);
	return (EXIT_OK);
}

INLINE static void prg_fix_352(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m352.chip);
}
INLINE static void chr_fix_352(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m352.chip);
}
INLINE static void mirroring_fix_352(void) {
	if (m352.chip & 0x01) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
