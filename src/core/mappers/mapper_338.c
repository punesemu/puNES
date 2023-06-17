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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_338(void);
INLINE static void chr_fix_338(void);
INLINE static void mirroring_fix_338(void);

INLINE static void tmp_fix_338(BYTE max, BYTE index, const WORD *ds);

struct _m338 {
	WORD reg;
} m338;
struct _m338tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m338tmp;

void map_init_338(void) {
	EXTCL_AFTER_MAPPER_INIT(338);
	EXTCL_CPU_WR_MEM(338);
	EXTCL_CPU_RD_MEM(338);
	EXTCL_SAVE_MAPPER(338);
	mapper.internal_struct[0] = (BYTE *)&m338;
	mapper.internal_struct_size[0] = sizeof(m338);

	if (info.reset >= HARD) {
		memset(&m338, 0x00, sizeof(m338));
	}

	if (info.reset == RESET) {
		if (m338tmp.ds_used) {
			m338tmp.index = (m338tmp.index + 1) % m338tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m338tmp, 0x00, sizeof(m338tmp));

		{
			static WORD ds[] = { 0x00 };

			tmp_fix_338(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_338(void) {
	prg_fix_338();
	chr_fix_338();
	mirroring_fix_338();
}
void extcl_cpu_wr_mem_338(WORD address, UNUSED(BYTE value)) {
	m338.reg = address;
	prg_fix_338();
	chr_fix_338();
	mirroring_fix_338();
}
BYTE extcl_cpu_rd_mem_338(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		switch (m338.reg & 0xFF0F) {
			case 0xF004:
				return (prgrom_size() <= S64K ? m338tmp.dipswitch[m338tmp.index] & 0x00FF : prgrom_rd(address));
			case 0xF008:
				return ((m338tmp.dipswitch[m338tmp.index] & 0xFF00) >> 8);
			default:
				return (prgrom_rd(address));
		}
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_338(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m338.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_338(void) {
	memmap_auto_16k(MMCPU(0x8000), m338.reg);
	memmap_auto_16k(MMCPU(0xC000), m338.reg);
}
INLINE static void chr_fix_338(void) {
	memmap_auto_8k(MMPPU(0x0000), m338.reg);
}
INLINE static void mirroring_fix_338(void) {
	if (m338.reg & 0x08) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}

INLINE static void tmp_fix_338(BYTE max, BYTE index, const WORD *ds) {
	m338tmp.ds_used = TRUE;
	m338tmp.max = max;
	m338tmp.index = index;
	m338tmp.dipswitch = ds;
}
