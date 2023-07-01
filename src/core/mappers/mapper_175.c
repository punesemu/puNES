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

INLINE static void prg_fix_175(void);
INLINE static void chr_fix_175(void);
INLINE static void mirroring_fix_175(void);

struct _m175 {
	BYTE reg[2];
	BYTE mirroring;
} m175;

void map_init_175(void) {
	EXTCL_AFTER_MAPPER_INIT(175);
	EXTCL_CPU_WR_MEM(175);
	EXTCL_CPU_RD_MEM(175);
	EXTCL_SAVE_MAPPER(175);

	if (info.reset >= HARD) {
		memset(&m175, 0x00, sizeof(m175));
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_175(void) {
	prg_fix_175();
	chr_fix_175();
	mirroring_fix_175();
}
void extcl_cpu_wr_mem_175(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m175.mirroring = value;
			mirroring_fix_175();
			return;
		case 0xA000:
			m175.reg[1] = value;
			return;
	}
}
BYTE extcl_cpu_rd_mem_175(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		if ((address >= 0xF000) && (m175.reg[0] != m175.reg[1])) {
			m175.reg[0] = m175.reg[1];
			prg_fix_175();
			chr_fix_175();
		}
		return (prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_175(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m175.reg);
	save_slot_ele(mode, slot, m175.mirroring);

	return (EXIT_OK);
}

INLINE static void prg_fix_175(void) {
	memmap_auto_16k(MMCPU(0x8000), m175.reg[0]);
	memmap_auto_16k(MMCPU(0xC000), m175.reg[0]);
}
INLINE static void chr_fix_175(void) {
	memmap_auto_8k(MMPPU(0x0000), m175.reg[0]);
}
INLINE static void mirroring_fix_175(void) {
	if (m175.mirroring & 0x04) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
