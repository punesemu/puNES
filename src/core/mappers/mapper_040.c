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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_040(void);
INLINE static void wram_fix_040(void);
INLINE static void mirroring_fix_040(void);

struct _m040 {
	BYTE reg[2];
	BYTE enabled;
	WORD count;
	BYTE delay;
} m040;

void map_init_040(void) {
	EXTCL_AFTER_MAPPER_INIT(040);
	EXTCL_CPU_WR_MEM(040);
	EXTCL_SAVE_MAPPER(040);
	EXTCL_CPU_EVERY_CYCLE(040);
	mapper.internal_struct[0] = (BYTE *)&m040;
	mapper.internal_struct_size[0] = sizeof(m040);

	memset(&m040, 0x00, sizeof(m040));
}
void extcl_after_mapper_init_040(void) {
	prg_fix_040();
	wram_fix_040();
	mirroring_fix_040();
}
void extcl_cpu_wr_mem_040(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			m040.enabled = FALSE;
			m040.count = 0;
			nes.c.irq.high &= ~EXT_IRQ;
			return;
		case 0xC000:
			if (info.mapper.submapper == 1) {
				m040.reg[1] = address & 0xFF;
				prg_fix_040();
			}
			return;
		case 0xA000:
			m040.enabled = TRUE;
			return;
		case 0xE000:
			m040.reg[0] = value;
			prg_fix_040();
			return;
	}
}
BYTE extcl_save_mapper_040(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m040.reg);
	save_slot_ele(mode, slot, m040.enabled);
	save_slot_ele(mode, slot, m040.count);
	save_slot_ele(mode, slot, m040.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_040(void) {
	if (m040.delay && !(--m040.delay)) {
		nes.c.irq.high |= EXT_IRQ;
	}
	if (m040.enabled && (++m040.count == 0x1000)) {
		m040.delay = 1;
	}
}

INLINE static void prg_fix_040(void) {
	if (m040.reg[1] & 0x08) {
		if (m040.reg[1] & 0x10)
			memmap_auto_32k(MMCPU(0x8000), 2 | (m040.reg[1] >> 6));
		else {
			memmap_auto_16k(MMCPU(0x8000), 4 | (m040.reg[1] >> 5));
			memmap_auto_16k(MMCPU(0xC000), 4 | (m040.reg[1] >> 5));
		}
	} else {
		memmap_auto_8k(MMCPU(0x8000), 4);
		memmap_auto_8k(MMCPU(0xA000), 5);
		memmap_auto_8k(MMCPU(0xC000), m040.reg[0] & 0x07);
		memmap_auto_8k(MMCPU(0xE000), 7);
	}
}
INLINE static void wram_fix_040(void) {
	memmap_prgrom_8k(MMCPU(0x6000), 6);
}
INLINE static void mirroring_fix_040(void) {
	if (m040.reg[1] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
