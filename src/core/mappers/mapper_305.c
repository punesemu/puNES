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

INLINE static void prg_fix_305(void);
INLINE static void wram_fix_305(void);

struct _m305 {
	BYTE reg[4];
} m305;

void map_init_305(void) {
	EXTCL_AFTER_MAPPER_INIT(305);
	EXTCL_CPU_WR_MEM(305);
	EXTCL_SAVE_MAPPER(305);
	map_internal_struct_init((BYTE *)&m305, sizeof(m305));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S2K);
		memmap_wram_region_init(0, S2K);
	}

	if (info.reset >= HARD) {
		memset(&m305, 0x00, sizeof(m305));
	}
}
void extcl_after_mapper_init_305(void) {
	prg_fix_305();
	wram_fix_305();
}
void extcl_cpu_wr_mem_305(UNUSED(BYTE nidx), WORD address, BYTE value) {
	m305.reg[(address & 0x1800) >> 11] = value;
	wram_fix_305();
}
BYTE extcl_save_mapper_305(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m305.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_305(void) {
	memmap_auto_2k(0, MMCPU(0x8000), 15);
	memmap_auto_2k(0, MMCPU(0x8800), 14);
	memmap_auto_2k(0, MMCPU(0x9000), 13);
	memmap_auto_2k(0, MMCPU(0x9800), 12);
	memmap_auto_2k(0, MMCPU(0xA000), 11);
	memmap_auto_2k(0, MMCPU(0xA800), 10);
	memmap_auto_2k(0, MMCPU(0xB000), 9);
	memmap_auto_2k(0, MMCPU(0xB800), 8);
	memmap_auto_2k(0, MMCPU(0xC000), 7);
	memmap_auto_2k(0, MMCPU(0xC800), 6);
	memmap_auto_2k(0, MMCPU(0xD000), 5);
	memmap_auto_2k(0, MMCPU(0xD800), 4);
	memmap_auto_2k(0, MMCPU(0xE000), 3);
	memmap_auto_2k(0, MMCPU(0xE800), 2);
	memmap_auto_2k(0, MMCPU(0xF000), 1);
	memmap_auto_2k(0, MMCPU(0xF800), 0);
}
INLINE static void wram_fix_305(void) {
	memmap_prgrom_2k(0, MMCPU(0x6000), m305.reg[0]);
	memmap_prgrom_2k(0, MMCPU(0x6800), m305.reg[1]);
	memmap_prgrom_2k(0, MMCPU(0x7000), m305.reg[2]);
	memmap_prgrom_2k(0, MMCPU(0x7800), m305.reg[3]);
}
