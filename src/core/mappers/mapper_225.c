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

INLINE static void prg_fix_225(void);
INLINE static void chr_fix_225(void);
INLINE static void mirroring_fix_225(void);

struct _m225 {
	WORD reg;
	BYTE scratch[4];
} m225;

void map_init_225(void) {
	EXTCL_AFTER_MAPPER_INIT(225);
	EXTCL_CPU_WR_MEM(225);
	EXTCL_CPU_RD_MEM(225);
	EXTCL_SAVE_MAPPER(225);
	mapper.internal_struct[0] = (BYTE *)&m225;
	mapper.internal_struct_size[0] = sizeof(m225);

	if (info.reset >= HARD) {
		memset(&m225, 0x00, sizeof(m225));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_225(void) {
	prg_fix_225();
	chr_fix_225();
	mirroring_fix_225();
}
void extcl_cpu_wr_mem_225(WORD address, UNUSED(BYTE value)) {
	if ((address >= 0x5000) && (address <= 0x5FFF) && (address & 0x0800)) {
		m225.scratch[address & 0x03] = value;
	} else if (address >= 0x8000) {
		m225.reg = address;
		prg_fix_225();
		chr_fix_225();
		mirroring_fix_225();
	}
}
BYTE extcl_cpu_rd_mem_225(WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (address & 0x800 ? m225.scratch[address & 0x03] : openbus);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_225(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m225.reg);
	save_slot_ele(mode, slot, m225.scratch);

	return (EXIT_OK);
}

INLINE static void prg_fix_225(void) {
	WORD bank = ((m225.reg & 0x4000) >> 8) | ((m225.reg & 0xFC0) >> 6);

	if (m225.reg & 0x1000) {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	}
}
INLINE static void chr_fix_225(void) {
	memmap_auto_8k(MMPPU(0x0000), (((m225.reg & 0x4000) >> 8) | (m225.reg & 0x3F)));
}
INLINE static void mirroring_fix_225(void) {
	if (m225.reg & 0x2000) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
