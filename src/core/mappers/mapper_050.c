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

INLINE static void prg_fix_050(void);
INLINE static void wram_fix_050(void);

struct _m050 {
	BYTE reg;
	struct _m50_irq {
		BYTE enabled;
		WORD count;
		BYTE delay;
	} irq;
} m050;

void map_init_050(void) {
	EXTCL_AFTER_MAPPER_INIT(050);
	EXTCL_CPU_WR_MEM(050);
	EXTCL_SAVE_MAPPER(050);
	EXTCL_CPU_EVERY_CYCLE(050);
	mapper.internal_struct[0] = (BYTE *)&m050;
	mapper.internal_struct_size[0] = sizeof(m050);

	if (info.reset >= HARD) {
		memset(&m050, 0x00, sizeof(m050));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_050(void) {
	prg_fix_050();
	wram_fix_050();
}
void extcl_cpu_wr_mem_050(WORD address, BYTE value) {
	if ((address <= 0x5FFF) && ((address & 0x0060) == 0x0020)) {
		if (address & 0x0100) {
			m050.irq.enabled = value & 0x01;
			if (!m050.irq.enabled) {
				m050.irq.count = 0;
				irq.high &= ~EXT_IRQ;
			}
			return;
		}
		m050.reg = value;
		prg_fix_050();
	}
}
BYTE extcl_save_mapper_050(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m050.reg);
	save_slot_ele(mode, slot, m050.irq.enabled);
	save_slot_ele(mode, slot, m050.irq.count);
	save_slot_ele(mode, slot, m050.irq.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_050(void) {
	if (m050.irq.delay && !(--m050.irq.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (m050.irq.enabled && (++m050.irq.count == 0x1000)) {
		m050.irq.delay = 1;
	}
}

INLINE static void prg_fix_050(void) {
	memmap_auto_8k(MMCPU(0x8000), 8);
	memmap_auto_8k(MMCPU(0xA000), 9);
	memmap_auto_8k(MMCPU(0xC000), ((m050.reg & 0x08) | ((m050.reg & 0x01) << 2) | ((m050.reg & 0x06) >> 1)));
	memmap_auto_8k(MMCPU(0xE000), 11);
}
INLINE static void wram_fix_050(void) {
	memmap_prgrom_8k(MMCPU(0x6000), 0x0F);
}
