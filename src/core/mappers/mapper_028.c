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

INLINE static void prg_fix_028(void);
INLINE static void chr_fix_028(void);
INLINE static void mirroring_fix_028(void);

INLINE static WORD prg_bank_028(WORD address);

struct _m028 {
	BYTE index;
	BYTE reg[4];
} m028;

void map_init_028(void) {
	EXTCL_AFTER_MAPPER_INIT(028);
	EXTCL_CPU_WR_MEM(028);
	EXTCL_SAVE_MAPPER(028);
	map_internal_struct_init((BYTE *)&m028, sizeof(m028));

	if (info.reset >= HARD) {
		memset(&m028, 0xFF, sizeof(m028));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_028(void) {
	prg_fix_028();
	chr_fix_028();
	mirroring_fix_028();
}
void extcl_cpu_wr_mem_028(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m028.index = ((value & 0x80) >> 6) | (value & 0x01);
		return;
	} else if (address >= 0x8000) {
		m028.reg[m028.index] = value;
		if (!(m028.index & 0x02) && !(m028.reg[2] & 0x02)) {
			m028.reg[2] = (m028.reg[2] & 0xFE) | ((value & 0x10) >> 4);
		}
		prg_fix_028();
		chr_fix_028();
		mirroring_fix_028();
		return;
	}
}
BYTE extcl_save_mapper_028(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m028.index);
	save_slot_ele(mode, slot, m028.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_028(void) {
	memmap_auto_16k(0, MMCPU(0x8000), prg_bank_028(0x8000));
	memmap_auto_16k(0, MMCPU(0xC000), prg_bank_028(0xC000));
}
INLINE static void chr_fix_028(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m028.reg[0] & 0x03));
}
INLINE static void mirroring_fix_028(void) {
	switch (m028.reg[2] & 0x03) {
		case 0:
			mirroring_SCR0(0);
			break;
		case 1:
			mirroring_SCR1(0);
			break;
		case 2:
			mirroring_V(0);
			break;
		case 3:
			mirroring_H(0);
			break;
	}
}

INLINE static WORD prg_bank_028(WORD address) {
	static const WORD bank_size_masks[4] = { 0x01, 0x03, 0x07, 0x0F };
	WORD cpu_a14 = (address >> 14) & 0x01;
	WORD outer_bank = m028.reg[3] << 1;
	WORD bank_mode = m028.reg[2] >> 2;
	WORD current_bank = m028.reg[1];
	WORD bank_size_mask = 0;

	if (((bank_mode ^ cpu_a14) & 0x03) == 0x02) {
		bank_mode = 0;
	}
	if ((bank_mode & 0x02) == 0) {
		current_bank = (current_bank << 1) | cpu_a14;
	}
	bank_size_mask = bank_size_masks[(bank_mode >> 2) & 0x03];
	return ((current_bank & bank_size_mask) | (outer_bank & ~bank_size_mask));
}
