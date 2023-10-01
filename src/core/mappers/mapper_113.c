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

INLINE static void prg_fix_113(void);
INLINE static void chr_fix_113(void);
INLINE static void mirroring_fix_113(void);

struct _m113 {
	BYTE reg;
} m113;

void map_init_113(void) {
	EXTCL_AFTER_MAPPER_INIT(113);
	EXTCL_CPU_WR_MEM(113);
	EXTCL_SAVE_MAPPER(113);
	mapper.internal_struct[0] = (BYTE *)&m113;
	mapper.internal_struct_size[0] = sizeof(m113);

	if (info.reset >= HARD) {
		memset(&m113, 0x00, sizeof(m113));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_113(void) {
	prg_fix_113();
	chr_fix_113();
	mirroring_fix_113();
}
void extcl_cpu_wr_mem_113(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if (address & 0x100) {
			m113.reg = value;
			prg_fix_113();
			chr_fix_113();
			mirroring_fix_113();
		}
	}
}
BYTE extcl_save_mapper_113(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m113.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_113(void) {
	memmap_auto_32k(0, MMCPU(0x8000), ((m113.reg & 0x38) >> 3));
}
INLINE static void chr_fix_113(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m113.reg & 0x40) >> 3) | (m113.reg & 0x07)));
}
INLINE static void mirroring_fix_113(void) {
	if (m113.reg & 0x80) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
