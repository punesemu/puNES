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

INLINE static void prg_fix_235(void);
INLINE static void chr_fix_235(void);
INLINE static void mirroring_fix_235(void);

struct _m235 {
	WORD reg[2];
	BYTE mode;
} m235;

void map_init_235(void) {
	EXTCL_AFTER_MAPPER_INIT(235);
	EXTCL_CPU_WR_MEM(235);
	EXTCL_SAVE_MAPPER(235);
	mapper.internal_struct[0] = (BYTE *)&m235;
	mapper.internal_struct_size[0] = sizeof(m235);

	memset(&m235, 0x00, sizeof(m235));

	if (prgrom_size() & S128K) {
		m235.mode = info.reset >= HARD ? 1 : !m235.mode;
	}
}
void extcl_after_mapper_init_235(void) {
	prg_fix_235();
	chr_fix_235();
	mirroring_fix_235();
}
void extcl_cpu_wr_mem_235(WORD address, BYTE value) {
	m235.reg[0] = address;
	m235.reg[1] = value;
	prg_fix_235();
	chr_fix_235();
	mirroring_fix_235();
}
BYTE extcl_save_mapper_235(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m235.reg);
	save_slot_ele(mode, slot, m235.mode);

	return (EXIT_OK);
}

INLINE static void prg_fix_235(void) {
	WORD bank = 0;

	if (m235.mode) {
		bank = prgrom_banks(S16K) & 0xC0;
		memmap_auto_16k(MMCPU(0x8000), (bank | (m235.reg[1] & 0x07)));
		memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
	} else {
		bank = ((m235.reg[0] & 0x300) >> 3) | (m235.reg[0] & 0x1F);
		if (bank >= prgrom_banks(S32K)) {
			memmap_disable_32k(MMCPU(0x8000));
		} else if (m235.reg[0] & 0x0800) {
			bank = (bank << 1) | ((m235.reg[0] & 0x1000) >> 12);
			memmap_auto_16k(MMCPU(0x8000), bank);
			memmap_auto_16k(MMCPU(0xC000), bank);
		} else {
			memmap_auto_32k(MMCPU(0x8000), bank);
		}
	}
}
INLINE static void chr_fix_235(void) {
	memmap_auto_8k(MMPPU(0x0000), 0);
}
INLINE static void mirroring_fix_235(void) {
	if (m235.mode) {
		mirroring_V();
	} else {
		if (m235.reg[0] & 0x0400) {
			mirroring_SCR0();
		} else if (m235.reg[0] & 0x2000) {
			// Horizontal mirroring also protects CHR-RAM as a side effect
			memmap_auto_wp_8k(MMPPU(0x0000), 0, TRUE, FALSE);
			mirroring_H();
		} else {
			mirroring_V();
		}
	}
}
