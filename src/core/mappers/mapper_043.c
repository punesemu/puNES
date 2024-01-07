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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_043(void);
INLINE static void wram_fix_043(void);

struct _m043 {
	BYTE reg;
	BYTE swap;
	struct _m043_irq {
		BYTE active;
		WORD count;
	} irq;
} m043;

void map_init_043(void) {
	EXTCL_AFTER_MAPPER_INIT(043);
	EXTCL_CPU_WR_MEM(043);
	EXTCL_SAVE_MAPPER(043);
	EXTCL_CPU_EVERY_CYCLE(043);
	map_internal_struct_init((BYTE *)&m043, sizeof(m043));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_wram_region_init(0, S2K);
	}

	if (info.reset >= HARD) {
		memset(&m043, 0x00, sizeof(m043));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_043(void) {
	prg_fix_043();
	wram_fix_043();
}
void extcl_cpu_wr_mem_043(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF1FF) {
		case 0x4022:
			m043.reg = value;
			prg_fix_043();
			return;
		case 0x4120:
			// Mr. Mary 2 (Unl)[!].nes
			m043.swap = value & 0x01;
			prg_fix_043();
			return;
		case 0x8122:
		case 0x4122:
			m043.irq.active = value & 0x01;
			m043.irq.count = 0;
			nes[nidx].c.irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_043(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m043.reg);
	save_slot_ele(mode, slot, m043.irq.active);
	save_slot_ele(mode, slot, m043.irq.count);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_043(BYTE nidx) {
	m043.irq.count++;
	if (m043.irq.active && (m043.irq.count >= 4096)) {
		m043.irq.active = 0;
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_043(void) {
	static const BYTE prg_e000[8] = { 4, 3, 4, 4, 4, 7, 5, 6 };

	memmap_auto_8k(0, MMCPU(0x8000), 1);
	memmap_auto_8k(0, MMCPU(0xA000), 0);
	memmap_auto_8k(0, MMCPU(0xC000), prg_e000[m043.reg & 0x07]);
	memmap_auto_8k(0, MMCPU(0xE000),  (m043.swap ? 8 : 9));
}
INLINE static void wram_fix_043(void) {
	WORD bank = ((0x10000 | (dipswitch.used ? dipswitch.value : 0x800)) / S2K);

	memmap_prgrom_2k(0, MMCPU(0x5000), bank);
	memmap_prgrom_2k(0, MMCPU(0x5800), bank);
	memmap_prgrom_8k(0, MMCPU(0x6000), m043.swap ? 0 : 2);
}
