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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_357(void);
INLINE static void wram_fix_357(void);
INLINE static void mirroring_fix_357(void);

INLINE static void tmp_fix_357(BYTE max, BYTE index, const BYTE *ds);

struct _m357 {
	BYTE reg[3];
	struct _m357_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m357;
struct _m357tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m357tmp;

void map_init_357(void) {
	EXTCL_AFTER_MAPPER_INIT(357);
	EXTCL_CPU_WR_MEM(357);
	EXTCL_SAVE_MAPPER(357);
	EXTCL_CPU_EVERY_CYCLE(357);
	mapper.internal_struct[0] = (BYTE *)&m357;
	mapper.internal_struct_size[0] = sizeof(m357);

	m357.reg[0] = 0;
	m357.reg[1] = 0;
	m357.reg[2] = 0;
	m357.irq.enable = 0;
	m357.irq.counter = 0;

	if (info.reset == RESET) {
		if (m357tmp.ds_used) {
			m357tmp.index = (m357tmp.index + 1) % m357tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (info.crc32.prg == 0x6C30D765) { // 4-in-1 (4602).nes
			static BYTE ds[4] = { 0x00, 0x08, 0x10, 0x18 };

			tmp_fix_357(LENGTH(ds), 0, &ds[0]);
		} else {
			static BYTE ds[] = { 0x00 };

			tmp_fix_357(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_357(void) {
	prg_fix_357();
	wram_fix_357();
	mirroring_fix_357();
}
void extcl_cpu_wr_mem_357(WORD address, BYTE value) {
	if (address >= 0x4000) {
		if (address & 0x8000) {
			m357.reg[0] = value & 0x07;
			prg_fix_357();
		}
		if ((address & 0x71FF) == 0x4022) {
			m357.reg[1] = value & 0x07;
			prg_fix_357();
		}
		if ((address & 0x71FF) == 0x4120) {
			m357.reg[2] = value & 0x01;
			prg_fix_357();
		}
		if ((address & 0xF1FF) == 0x4122) {
			m357.irq.enable = value & 0x01;
			m357.irq.counter = 0;
			irq.high &= ~EXT_IRQ;
		}
	}
}
BYTE extcl_save_mapper_357(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m357.reg);
	save_slot_ele(mode, slot, m357.irq.enable);
	save_slot_ele(mode, slot, m357.irq.counter);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_357(void) {
	if (m357.irq.enable) {
		m357.irq.counter++;
		if (m357.irq.counter == 0x1000) {
			m357.irq.counter = 0;
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_357(void) {
	if (!m357tmp.dipswitch[m357tmp.index]) {
		static BYTE banks[2][8] = {
				{ 4, 3, 5, 3, 6, 3, 7, 3 },
				{ 1, 1, 5, 1, 4, 1, 5, 1 }
		};

		memmap_auto_8k(MMCPU(0x8000), (m357.reg[2] ? 0 : 1));
		memmap_auto_8k(MMCPU(0xA000), 0);
		memmap_auto_8k(MMCPU(0xC000), banks[m357.reg[2]][m357.reg[1]]);
		memmap_auto_8k(MMCPU(0xE000), (m357.reg[2] ? 8 : 10));
	} else {
		memmap_auto_16k(MMCPU(0x8000), (m357tmp.dipswitch[m357tmp.index] | m357.reg[0]));
		memmap_auto_16k(MMCPU(0xC000), (m357tmp.dipswitch[m357tmp.index] | 0x07));
	}
}
INLINE static void wram_fix_357(void) {
	if (!m357tmp.dipswitch[m357tmp.index]) {
		memmap_prgrom_8k(MMCPU(0x6000), m357.reg[2] ? 0 : 2);
	}
}
INLINE static void mirroring_fix_357(void) {
	if (m357tmp.dipswitch[m357tmp.index] == 0x18) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_357(BYTE max, BYTE index, const BYTE *ds) {
	m357tmp.ds_used = TRUE;
	m357tmp.max = max;
	m357tmp.index = index;
	m357tmp.dipswitch = ds;
}
