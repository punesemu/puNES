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

INLINE static void prg_fix_186(void);
INLINE static void wram_fix_186(void);

struct _m186 {
	BYTE reg[4];
} m186;

void map_init_186(void) {
	EXTCL_AFTER_MAPPER_INIT(186);
	EXTCL_CPU_WR_MEM(186);
	EXTCL_CPU_RD_MEM(186);
	EXTCL_SAVE_MAPPER(186);
	mapper.internal_struct[0] = (BYTE *)&m186;
	mapper.internal_struct_size[0] = sizeof(m186);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_wram_region_init(0, S1K);
	}

	if (info.reset >= HARD) {
		memset(&m186, 0x00, sizeof(m186));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_186(void) {
	prg_fix_186();
	wram_fix_186();
}
void extcl_cpu_wr_mem_186(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x4200) && (address <= 0x4203)) {
		m186.reg[address & 0x03] = value;
		prg_fix_186();
		wram_fix_186();
	}
}
BYTE extcl_cpu_rd_mem_186(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	switch (address) {
		case 0x4200:
		case 0x4201:
		case 0x4203:
			return (0x00);
		case 0x4202:
			return (0x40);
		default:
			return (wram_rd(nidx, address));
	}
}
BYTE extcl_save_mapper_186(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m186.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_186(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m186.reg[1]);
	memmap_auto_16k(0, MMCPU(0xC000), 0);
}
INLINE static void wram_fix_186(void) {
	memmap_auto_custom_size(0, MMCPU(0x4400), 0, (size_t)(0x400 * 3));
	memmap_auto_8k(0, MMCPU(0x6000), m186.reg[0] >> 6);
}
