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
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
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
struct _m043tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m043tmp;

void map_init_043(void) {
	EXTCL_AFTER_MAPPER_INIT(043);
	EXTCL_CPU_WR_MEM(043);
	EXTCL_SAVE_MAPPER(043);
	EXTCL_CPU_EVERY_CYCLE(043);
	mapper.internal_struct[0] = (BYTE *)&m043;
	mapper.internal_struct_size[0] = sizeof(m043);

	if (info.reset >= HARD) {
		memset(&m043, 0x00, sizeof(m043));
	}

	// vale come promemoria per quando implementero' il dipswitch letto dal file
	m043tmp.ds_used = FALSE;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_043(void) {
	prg_fix_043();
	wram_fix_043();
}
void extcl_cpu_wr_mem_043(WORD address, BYTE value) {
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
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_043(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m043.reg);
	save_slot_ele(mode, slot, m043.irq.active);
	save_slot_ele(mode, slot, m043.irq.count);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_043();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_043(void) {
	m043.irq.count++;
	if (m043.irq.active && (m043.irq.count >= 4096)) {
		m043.irq.active = 0;
		irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_043(void) {
	static const BYTE prg_e000[8] = { 4, 3, 4, 4, 4, 7, 5, 6 };

	memmap_auto_8k(MMCPU(0x8000), 1);
	memmap_auto_8k(MMCPU(0xA000), 0);
	memmap_auto_8k(MMCPU(0xC000), prg_e000[m043.reg & 0x07]);
	memmap_auto_8k(MMCPU(0xE000),  (m043.swap ? 8 : 9));
}
INLINE static void wram_fix_043(void) {
	memmap_prgrom_2k(MMCPU(0x5000), m043tmp.ds_used ? 32 : 33);
	memmap_prgrom_2k(MMCPU(0x5800), m043tmp.ds_used ? 32 : 33);
	memmap_prgrom_8k(MMCPU(0x6000), m043.swap ? 0 : 2);
}
