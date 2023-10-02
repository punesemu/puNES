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

INLINE static void prg_fix_303(void);
INLINE static void mirroring_fix_303(void);

struct _m303 {
	BYTE reg[2];
	struct _m303_irq {
		BYTE active;
		WORD count;
	} irq;
} m303;

void map_init_303(void) {
	EXTCL_AFTER_MAPPER_INIT(303);
	EXTCL_CPU_WR_MEM(303);
	EXTCL_CPU_RD_MEM(303);
	EXTCL_SAVE_MAPPER(303);
	EXTCL_CPU_EVERY_CYCLE(303);
	mapper.internal_struct[0] = (BYTE *)&m303;
	mapper.internal_struct_size[0] = sizeof(m303);

	if (info.reset >= HARD) {
		memset(&m303, 0x00, sizeof(m303));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_303(void) {
	prg_fix_303();
	mirroring_fix_303();
}
void extcl_cpu_wr_mem_303(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if ((address & 0xFF00) == 0x4A00) {
			m303.reg[0] = ((address >> 4) & 0x04) | ((address & 0x0C) >> 2);
			return;
		} else if ((address & 0xFF00) == 0x5100) {
			prg_fix_303();
			return;
		} else if (address == 0x4020) {
			nes[nidx].c.irq.high &= ~EXT_IRQ;
			m303.irq.count = (m303.irq.count & 0xFF00) | value;
			return;
		} else if (address == 0x4021) {
			nes[nidx].c.irq.high &= ~EXT_IRQ;
			m303.irq.count = (value << 8) | (m303.irq.count & 0x00FF);
			m303.irq.active = 1;
			return;
		} else if (address == 0x4025) {
			m303.reg[1] = value;
			mirroring_fix_303();
			return;
		}
	}
}
BYTE extcl_cpu_rd_mem_303(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address == 0x4030) {
		openbus = (nes[nidx].c.irq.high & EXT_IRQ) ? 1 : 0;
		nes[nidx].c.irq.high &= ~EXT_IRQ;
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_303(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m303.reg);
	save_slot_ele(mode, slot, m303.irq.active);
	save_slot_ele(mode, slot, m303.irq.count);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_303(BYTE nidx) {
	if (m303.irq.active) {
		if (--m303.irq.count == 0) {
			m303.irq.active = 0;
			nes[nidx].c.irq.delay = TRUE;
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_303(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m303.reg[0]);
	memmap_auto_16k(0, MMCPU(0xC000), 0x02);
}
INLINE static void mirroring_fix_303(void) {
	if (m303.reg[1] & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
