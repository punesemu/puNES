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

INLINE static void prg_fix_301(void);
INLINE static void mirroring_fix_301(void);

struct _m301 {
	WORD reg;
} m301;

void map_init_301(void) {
	EXTCL_AFTER_MAPPER_INIT(301);
	EXTCL_CPU_WR_MEM(301);
	EXTCL_CPU_RD_MEM(301);
	EXTCL_SAVE_MAPPER(301);
	map_internal_struct_init((BYTE *)&m301, sizeof(m301));

	memset(&m301, 0x00, sizeof(m301));

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_301(void) {
	prg_fix_301();
	mirroring_fix_301();
}
void extcl_cpu_wr_mem_301(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m301.reg = address;
	prg_fix_301();
	mirroring_fix_301();
}
BYTE extcl_cpu_rd_mem_301(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m301.reg & 0x100 ? prgrom_rd(nidx, (address & 0xFFFE) | dipswitch.value) : prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_301(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m301.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_301(void) {
	WORD bank = (m301.reg & 0x7C) >> 2;
	WORD mask = m301.reg & 0x80 ? 0x07 : 0x00;

	memmap_auto_16k(0, MMCPU(0x8000), bank);
	memmap_auto_16k(0, MMCPU(0xC000), ((bank & ~mask) | (((m301.reg & 0x200) >> 9) * 7)));
}
INLINE static void mirroring_fix_301(void) {
	if (m301.reg & 0x02) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
