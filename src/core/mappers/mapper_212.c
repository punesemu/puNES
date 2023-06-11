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

INLINE static void prg_fix_212(void);
INLINE static void chr_fix_212(void);
INLINE static void mirroring_fix_212(void);

struct _m212 {
	WORD reg;
} m212;

void map_init_212(void) {
	EXTCL_AFTER_MAPPER_INIT(212);
	EXTCL_CPU_WR_MEM(212);
	EXTCL_CPU_RD_MEM(212);
	EXTCL_SAVE_MAPPER(212);

	if (info.reset >= HARD) {
		memset(&m212, 0x00, sizeof(m212));
	}
}
void extcl_after_mapper_init_212(void) {
	prg_fix_212();
	chr_fix_212();
	mirroring_fix_212();
}
void extcl_cpu_wr_mem_212(WORD address, UNUSED(BYTE value)) {
	m212.reg = address;
	prg_fix_212();
	chr_fix_212();
	mirroring_fix_212();
}
BYTE extcl_cpu_rd_mem_212(WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (openbus | (address & 0x10 ? 0x00 : 0x80));
	}
	return (openbus);
}
BYTE extcl_save_mapper_212(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m212.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_212(void) {
	if (m212.reg & 0x4000) {
		memmap_auto_32k(MMCPU(0x8000), (m212.reg >> 1));
	} else {
		memmap_auto_16k(MMCPU(0x8000), m212.reg);
		memmap_auto_16k(MMCPU(0xC000), m212.reg);
	}
}
INLINE static void chr_fix_212(void) {
	memmap_auto_8k(MMPPU(0x0000), m212.reg);
}
INLINE static void mirroring_fix_212(void) {
	if (m212.reg & 0x08) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
