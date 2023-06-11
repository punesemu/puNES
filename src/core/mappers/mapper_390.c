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

INLINE static void prg_fix_390(void);
INLINE static void chr_fix_390(void);
INLINE static void mirroring_fix_390(void);

INLINE static void tmp_fix_390(BYTE max, BYTE index, const BYTE *ds);

struct _m390 {
	WORD reg[3];
} m390;
struct _m390tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m390tmp;

void map_init_390(void) {
	EXTCL_AFTER_MAPPER_INIT(390);
	EXTCL_CPU_WR_MEM(390);
	EXTCL_CPU_RD_MEM(390);
	EXTCL_SAVE_MAPPER(390);
	mapper.internal_struct[0] = (BYTE *)&m390;
	mapper.internal_struct_size[0] = sizeof(m390);

	memset(&m390, 0x00, sizeof(m390));

	if (info.reset == RESET) {
		if (m390tmp.ds_used) {
			m390tmp.index = (m390tmp.index + 1) % m390tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (info.crc32.prg == 0xF92EFDE7) { // 150-in-1.nes
			static BYTE ds[] = { 0x0D };

			tmp_fix_390(LENGTH(ds), 0, &ds[0]);
		} else {
			static BYTE ds[] = { 0x00 };

			tmp_fix_390(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_390(void) {
	prg_fix_390();
	chr_fix_390();
	mirroring_fix_390();
}
void extcl_cpu_wr_mem_390(WORD address, UNUSED(BYTE value)) {
	m390.reg[(address >> 14) & 0x01] = address & 0xFF;
	prg_fix_390();
	chr_fix_390();
	mirroring_fix_390();
}
BYTE extcl_cpu_rd_mem_390(WORD address, BYTE openbus) {
	if ((address >= 0x8000) && ((m390.reg[1] & 0x30) == 0x10)) {
		return (prgrom_rd((address |  m390tmp.dipswitch[m390tmp.index])));
	}
	return (openbus);
}
BYTE extcl_save_mapper_390(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m390.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_390(void) {
	WORD bank = m390.reg[1];

	switch(m390.reg[1] & 0x30) {
		case 0x00:
		case 0x10:
			memmap_auto_16k(MMCPU(0x8000), bank);
			memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
			return;
		case 0x20:
			memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
			return;
		case 0x30:
			memmap_auto_16k(MMCPU(0x8000), bank);
			memmap_auto_16k(MMCPU(0xC000), bank);
			return;
	}
}
INLINE static void chr_fix_390(void) {
	memmap_auto_8k(MMPPU(0x0000), m390.reg[0]);
}
INLINE static void mirroring_fix_390(void) {
	if (m390.reg[0] & 0x20) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}

INLINE static void tmp_fix_390(BYTE max, BYTE index, const BYTE *ds) {
	m390tmp.ds_used = TRUE;
	m390tmp.max = max;
	m390tmp.index = index;
	m390tmp.dipswitch = ds;
}
