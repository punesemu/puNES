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

INLINE static void prg_fix_301(void);
INLINE static void mirroring_fix_301(void);

INLINE static void tmp_fix_301(BYTE max, BYTE index, const WORD *ds);

struct _m301 {
	WORD reg;
} m301;
struct _m301tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m301tmp;

void map_init_301(void) {
	EXTCL_AFTER_MAPPER_INIT(301);
	EXTCL_CPU_WR_MEM(301);
	EXTCL_CPU_RD_MEM(301);
	EXTCL_SAVE_MAPPER(301);
	mapper.internal_struct[0] = (BYTE *)&m301;
	mapper.internal_struct_size[0] = sizeof(m301);

	memset(&m301, 0x00, sizeof(m301));

	if (info.reset == RESET) {
		if (m301tmp.ds_used) {
			m301tmp.index = (m301tmp.index + 1) % m301tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m301tmp, 0x00, sizeof(m301tmp));

		{
			static WORD ds[] = { 0x00 };

			tmp_fix_301(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_301(void) {
	prg_fix_301();
	mirroring_fix_301();
}
void extcl_cpu_wr_mem_301(WORD address, UNUSED(BYTE value)) {
	m301.reg = address;
	prg_fix_301();
	mirroring_fix_301();
}
BYTE extcl_cpu_rd_mem_301(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m301.reg & 0x100 ? prgrom_rd((address & 0xFFFE) | m301tmp.dipswitch[m301tmp.index]) : prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_301(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m301.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_301(void) {
	WORD bank = (m301.reg & 0x7C) >> 2;
	WORD mask = m301.reg & 0x80 ? 0x07 : 0x00;

	memmap_auto_16k(MMCPU(0x8000), bank);
	memmap_auto_16k(MMCPU(0xC000), ((bank & ~mask) | (((m301.reg & 0x200) >> 9) * 7)));
}
INLINE static void mirroring_fix_301(void) {
	if (m301.reg & 0x02) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_301(BYTE max, BYTE index, const WORD *ds) {
	m301tmp.ds_used = TRUE;
	m301tmp.max = max;
	m301tmp.index = index;
	m301tmp.dipswitch = ds;
}
