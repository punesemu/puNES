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

#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_059(void);
INLINE static void chr_fix_059(void);
INLINE static void mirroring_fix_059(void);

struct _m059 {
	WORD reg;
} m059;

void map_init_059(void) {
	EXTCL_AFTER_MAPPER_INIT(059);
	EXTCL_CPU_WR_MEM(059);
	EXTCL_CPU_RD_MEM(059);
	EXTCL_SAVE_MAPPER(059);
	map_internal_struct_init((BYTE *)&m059, sizeof(m059));

	m059.reg = 0;

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_059(void) {
	prg_fix_059();
	chr_fix_059();
	mirroring_fix_059();
}
void extcl_cpu_wr_mem_059(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	if (!(m059.reg & 0x0200)) {
		m059.reg = address;
		prg_fix_059();
		chr_fix_059();
		mirroring_fix_059();
	}
}
BYTE extcl_cpu_rd_mem_059(BYTE nidx, WORD address, BYTE openbus) {
	if (address >= 0x8000) {
		return (m059.reg & 0x0100 ? (openbus & 0xFC) | dipswitch.value : prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_059(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m059.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_059(void) {
	if (m059.reg & 0x80) {
		memmap_auto_16k(0, MMCPU(0x8000), ((m059.reg & 0xF0) >> 4));
		memmap_auto_16k(0, MMCPU(0xC000), ((m059.reg & 0xF0) >> 4));
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), ((m059.reg & 0xF0) >> 5));
	}
}
INLINE static void chr_fix_059(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m059.reg & 0x0F));
}
INLINE static void mirroring_fix_059(void) {
	if (m059.reg & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
