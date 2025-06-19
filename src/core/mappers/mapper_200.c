/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_200(void);
INLINE static void chr_fix_200(void);
INLINE static void mirroring_fix_200(void);

struct _m200 {
	WORD reg;
} m200;

void map_init_200(void) {
	EXTCL_AFTER_MAPPER_INIT(200);
	EXTCL_CPU_WR_MEM(200);
	EXTCL_CPU_RD_MEM(200);
	EXTCL_SAVE_MAPPER(200);

	if (info.reset >= HARD) {
		memset(&m200, 0x00, sizeof(m200));
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_200(void) {
	prg_fix_200();
	chr_fix_200();
	mirroring_fix_200();
}
void extcl_cpu_wr_mem_200(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m200.reg = address;
	prg_fix_200();
	chr_fix_200();
	mirroring_fix_200();
}
BYTE extcl_cpu_rd_mem_200(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		switch (m200.reg & 0xFF0F) {
			case 0xF004:
				return (prgrom_size() <= S64K ? dipswitch.value & 0x00FF : prgrom_rd(nidx, address));
			case 0xF008:
				return ((dipswitch.value & 0xFF00) >> 8);
			default:
				return (prgrom_rd(nidx, address));
		}
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_200(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m200.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_200(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m200.reg);
	memmap_auto_16k(0, MMCPU(0xC000), m200.reg);
}
INLINE static void chr_fix_200(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m200.reg);
}
INLINE static void mirroring_fix_200(void) {
	if (m200.reg & (info.mapper.submapper == 1 ? 0x04 : 0x08)) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
