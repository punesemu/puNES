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

INLINE static void prg_fix_304(void);
INLINE static void wram_fix_304(void);

struct _m304 {
	BYTE reg;
	struct _m304_irq {
		BYTE active;
		WORD count;
	} irq;
} m304;

void map_init_304(void) {
	EXTCL_AFTER_MAPPER_INIT(304);
	EXTCL_CPU_WR_MEM(304);
	EXTCL_CPU_RD_MEM(304);
	EXTCL_SAVE_MAPPER(304);
	EXTCL_CPU_EVERY_CYCLE(304);
	mapper.internal_struct[0] = (BYTE *)&m304;
	mapper.internal_struct_size[0] = sizeof(m304);

	if (info.reset >= HARD) {
		memset(&m304, 0x00, sizeof(m304));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_304(void) {
	prg_fix_304();
	wram_fix_304();
}
void extcl_cpu_wr_mem_304(WORD address, BYTE value) {
	if (address == 0x4027) {
		m304.reg = value & 0x01;
		wram_fix_304();
	} else if (address == 0x4068) {
		m304.irq.active = value & 0x01;
		m304.irq.count = 0;
		cpudata.irq.high &= ~EXT_IRQ;
	}
}
BYTE extcl_cpu_rd_mem_304(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4020) && (address <= 0x4FFF)) {
		return (0xFF);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_304(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m304.reg);
	save_slot_ele(mode, slot, m304.irq.active);
	save_slot_ele(mode, slot, m304.irq.count);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_304(void) {
	if (m304.irq.active) {
		if (m304.irq.count < 5750) {
			m304.irq.count++;
		} else {
			cpudata.irq.high |= EXT_IRQ;
			m304.irq.active = 0;
		}
	}
}

INLINE static void prg_fix_304(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
INLINE static void wram_fix_304(void) {
	memmap_prgrom_8k(MMCPU(0x6000), (m304.reg | 0x04));
}
