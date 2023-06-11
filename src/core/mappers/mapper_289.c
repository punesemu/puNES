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

INLINE static void prg_fix_289(void);
INLINE static void chr_fix_289(void);
INLINE static void mirroring_fix_289(void);

INLINE static void tmp_fix_289(BYTE max, BYTE index, const WORD *ds);

struct _m289 {
	BYTE reg[3];
} m289;
struct _m289tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m289tmp;

void map_init_289(void) {
	EXTCL_AFTER_MAPPER_INIT(289);
	EXTCL_CPU_WR_MEM(289);
	EXTCL_SAVE_MAPPER(289);
	mapper.internal_struct[0] = (BYTE *)&m289;
	mapper.internal_struct_size[0] = sizeof(m289);

	memset(&m289, 0x00, sizeof(m289));

	if (info.reset == RESET) {
		if (m289tmp.ds_used) {
			m289tmp.index = (m289tmp.index + 1) % m289tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m289tmp, 0x00, sizeof(m289tmp));

		{
			static WORD ds[] = { 0x00 };

			tmp_fix_289(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_289(void) {
	prg_fix_289();
	chr_fix_289();
	mirroring_fix_289();
}
void extcl_cpu_wr_mem_289(WORD address, BYTE value) {
	switch (address & 0xE289) {
		case 0x6000:
		case 0x6001:
		case 0x7000:
		case 0x7001:
			m289.reg[address & 0x01] = value & 0x7F;
			prg_fix_289();
			chr_fix_289();
			mirroring_fix_289();
			break;
		case 0x8000:
		case 0x8001:
		case 0xA000:
		case 0xA001:
		case 0xC000:
		case 0xC001:
		case 0xE000:
		case 0xE001:
			m289.reg[2] = value;
			prg_fix_289();
			mirroring_fix_289();
			break;
	}
}
BYTE extcl_cpu_rd_mem_289(WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return ((openbus & 0xFC) | (m289tmp.dipswitch[m289tmp.index] & 0x03));
	}
	return (openbus);
}
BYTE extcl_save_mapper_289(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m289.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_289(void) {
	WORD bank[2];

	if (m289.reg[0] & 0x02) {
		bank[0] = (m289.reg[1] & ~0x07) | (m289.reg[0] & 0x01 ? 0x07 : (m289.reg[2] & 0x07));
		bank[1] = m289.reg[1] | 0x07;
	} else {
		bank[0] = m289.reg[1] & ~(m289.reg[0] & 0x01);
		bank[1] = m289.reg[1] | (m289.reg[0] & 0x01);
	}
	memmap_auto_16k(MMCPU(0x8000), bank[0]);
	memmap_auto_16k(MMCPU(0xC000), bank[1]);
}
INLINE static void chr_fix_289(void) {
	memmap_vram_wp_8k(MMPPU(0x0000), 0, TRUE, !(m289.reg[0] & 0x04));
}
INLINE static void mirroring_fix_289(void) {
	if (m289.reg[0] & 0x08) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_289(BYTE max, BYTE index, const WORD *ds) {
	m289tmp.ds_used = TRUE;
	m289tmp.max = max;
	m289tmp.index = index;
	m289tmp.dipswitch = ds;
}
