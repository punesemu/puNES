/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_031(void);

struct _m031 {
	BYTE reg[8];
} m031;

void map_init_031(void) {
	EXTCL_AFTER_MAPPER_INIT(031);
	EXTCL_CPU_WR_MEM(031);
	EXTCL_SAVE_MAPPER(031);
	map_internal_struct_init((BYTE *)&m031, sizeof(m031));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S4K);
	}

	if (info.reset >= HARD) {
		m031.reg[0] = 0xF8;
		m031.reg[1] = 0xF9;
		m031.reg[2] = 0xFA;
		m031.reg[3] = 0xFB;
		m031.reg[4] = 0xFC;
		m031.reg[5] = 0xFD;
		m031.reg[6] = 0xFE;
		m031.reg[7] = 0xFF;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_031(void) {
	prg_fix_031();
}
void extcl_cpu_wr_mem_031(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m031.reg[address & 0x07] = value;
		prg_fix_031();
	}
}
BYTE extcl_save_mapper_031(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m031.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_031(void) {
	memmap_auto_4k(0, MMCPU(0x8000), m031.reg[0]);
	memmap_auto_4k(0, MMCPU(0x9000), m031.reg[1]);
	memmap_auto_4k(0, MMCPU(0xA000), m031.reg[2]);
	memmap_auto_4k(0, MMCPU(0xB000), m031.reg[3]);
	memmap_auto_4k(0, MMCPU(0xC000), m031.reg[4]);
	memmap_auto_4k(0, MMCPU(0xD000), m031.reg[5]);
	memmap_auto_4k(0, MMCPU(0xE000), m031.reg[6]);
	memmap_auto_4k(0, MMCPU(0xF000), m031.reg[7]);
}
