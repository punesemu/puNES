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

#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_125(void);
INLINE static void wram_fix_125(void);

struct _m125 {
	BYTE reg;
} m125;

void map_init_125(void) {
	EXTCL_AFTER_MAPPER_INIT(125);
	EXTCL_CPU_WR_MEM(125);
	EXTCL_SAVE_MAPPER(125);
	mapper.internal_struct[0] = (BYTE *)&m125;
	mapper.internal_struct_size[0] = sizeof(m125);

	if (info.reset >= HARD) {
		m125.reg = 0;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_125(void) {
	prg_fix_125();
	wram_fix_125();
}
void extcl_cpu_wr_mem_125(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		m125.reg = value;
		wram_fix_125();
		return;
	}
}
BYTE extcl_save_mapper_125(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m125.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_125(void) {
	memmap_auto_8k(MMCPU(0x8000), 0xFC);
	memmap_auto_8k(MMCPU(0xA000), 0xFD);
	memmap_wram_8k(MMCPU(0xC000), 0);
	memmap_auto_8k(MMCPU(0xE000), 0xFF);
}
INLINE static void wram_fix_125(void) {
	memmap_prgrom_8k(MMCPU(0x6000), m125.reg);
}
