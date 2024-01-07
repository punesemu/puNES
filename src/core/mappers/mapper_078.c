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

INLINE static void prg_fix_078(void);
INLINE static void chr_fix_078(void);
INLINE static void mirroring_fix_078(void);

struct _m078 {
	BYTE reg;
} m078;

void map_init_078(void) {
	EXTCL_AFTER_MAPPER_INIT(078);
	EXTCL_CPU_WR_MEM(078);
	EXTCL_SAVE_MAPPER(078);
	map_internal_struct_init((BYTE *)&m078, sizeof(m078));

	if (info.reset >= HARD) {
		memset(&m078, 0x00, sizeof(m078));
	}
}
void extcl_after_mapper_init_078(void) {
	prg_fix_078();
	chr_fix_078();
	mirroring_fix_078();
}
void extcl_cpu_wr_mem_078(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	m078.reg = value & prgrom_rd(nidx, address);
	prg_fix_078();
	chr_fix_078();
	mirroring_fix_078();
}
BYTE extcl_save_mapper_078(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m078.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_078(void) {
	memmap_auto_16k(0, MMCPU(0x8000), (m078.reg & 0x07));
	memmap_auto_16k(0, MMCPU(0xC000), 0x0F);
}
INLINE static void chr_fix_078(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m078.reg >> 4));
}
INLINE static void mirroring_fix_078(void) {
	if (info.mapper.submapper == 3) {
		if (m078.reg & 0x08) {
			mirroring_V(0);
		} else {
			mirroring_H(0);
		}
	} else if (m078.reg & 0x08) {
		mirroring_SCR1(0);
	} else {
		mirroring_SCR0(0);
	}
}
