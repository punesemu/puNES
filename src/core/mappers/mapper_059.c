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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_059(void);
INLINE static void chr_fix_059(void);
INLINE static void mirroring_fix_059(void);

INLINE static void tmp_fix_059(BYTE max, BYTE index, const BYTE *ds);

struct _m059 {
	WORD reg;
} m059;
struct _m059tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m059tmp;

void map_init_059(void) {
	EXTCL_AFTER_MAPPER_INIT(059);
	EXTCL_CPU_WR_MEM(059);
	EXTCL_CPU_RD_MEM(059);
	EXTCL_SAVE_MAPPER(059);
	mapper.internal_struct[0] = (BYTE *)&m059;
	mapper.internal_struct_size[0] = sizeof(m059);

	m059.reg = 0;

	if (info.reset == RESET) {
		if (m059tmp.ds_used) {
			m059tmp.index = (m059tmp.index + 1) % m059tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m059tmp, 0x00, sizeof(m059tmp));

		if (info.crc32.prg == 0xED831F98) { // (VT-104) 2000 Super Aladdin.nes
			static BYTE ds[] = {0x00, 0x01, 0x02};

			tmp_fix_059(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x30FDFBA7) { // (WQ5209) 1998 Super 64-in-1.nes
			static BYTE ds[] = { 0x00, 0x02, 0x01, 0x03 };

			tmp_fix_059(LENGTH(ds), 0, &ds[0]);
		} else {
			static BYTE ds[] = { 0x00 };

			tmp_fix_059(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_059(void) {
	prg_fix_059();
	chr_fix_059();
	mirroring_fix_059();
}
void extcl_cpu_wr_mem_059(WORD address, UNUSED(BYTE value)) {
	if (!(m059.reg & 0x0200)) {
		m059.reg = address;
		prg_fix_059();
		chr_fix_059();
		mirroring_fix_059();
	}
}
BYTE extcl_cpu_rd_mem_059(WORD address, BYTE openbus) {
	if ((address >= 0x8000) && (m059.reg & 0x0100)) {
		return ((openbus & 0xFC) | m059tmp.dipswitch[m059tmp.index]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_059(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m059.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_059(void) {
	if (m059.reg & 0x80) {
		memmap_auto_16k(MMCPU(0x8000), ((m059.reg & 0xF0) >> 4));
		memmap_auto_16k(MMCPU(0xC000), ((m059.reg & 0xF0) >> 4));
	} else {
		memmap_auto_32k(MMCPU(0x8000), ((m059.reg & 0xF0) >> 5));
	}
}
INLINE static void chr_fix_059(void) {
	memmap_auto_8k(MMPPU(0x0000), (m059.reg & 0x0F));
}
INLINE static void mirroring_fix_059(void) {
	if (m059.reg & 0x08) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_059(BYTE max, BYTE index, const BYTE *ds) {
	m059tmp.ds_used = TRUE;
	m059tmp.max = max;
	m059tmp.index = index;
	m059tmp.dipswitch = ds;
}
