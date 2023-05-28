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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_233(void);
INLINE static void mirroring_fix_233(void);

struct _m233 {
	BYTE reg;
	BYTE reset;
} m233;

void map_init_233(void) {
	EXTCL_AFTER_MAPPER_INIT(233);
	EXTCL_CPU_WR_MEM(233);
	EXTCL_SAVE_MAPPER(233);
	mapper.internal_struct[0] = (BYTE *)&m233;
	mapper.internal_struct_size[0] = sizeof(m233);

	if (info.reset >= HARD) {
		m233.reg = 0;
	}

	if (info.reset == RESET) {
		m233.reset ^= 0x20;
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m233.reset = 0;
	}
}
void extcl_after_mapper_init_233(void) {
	prg_fix_233();
	mirroring_fix_233();
}
void extcl_cpu_wr_mem_233(UNUSED(WORD address), BYTE value) {
	m233.reg = value;
	prg_fix_233();
	mirroring_fix_233();
}
BYTE extcl_save_mapper_233(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m233.reg);
	save_slot_ele(mode, slot, m233.reset);

	return (EXIT_OK);
}

INLINE static void prg_fix_233(void) {
	WORD bank = (m233.reg & 0x1F) | m233.reset;

	if (m233.reg & 0x20) {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	} else {
		bank >>= 1;
		memmap_auto_32k(MMCPU(0x8000), bank);
	}
}
INLINE static void mirroring_fix_233(void) {
	switch (m233.reg >> 6) {
		case 0:
			mirroring_SCR0();
			return;
		case 1:
			mirroring_V();
			return;
		case 2:
			mirroring_H();
			return;
		case 3:
			mirroring_SCR1();
			return;
	}
}
