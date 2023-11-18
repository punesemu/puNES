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

INLINE static void prg_fix_368(void);
INLINE static void wram_fix_368(void);

struct _m368 {
	BYTE reg[2];
	struct _m368_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m368;

void map_init_368(void) {
	EXTCL_AFTER_MAPPER_INIT(368);
	EXTCL_CPU_WR_MEM(368);
	EXTCL_CPU_RD_MEM(368);
	EXTCL_SAVE_MAPPER(368);
	EXTCL_CPU_EVERY_CYCLE(368);
	mapper.internal_struct[0] = (BYTE *)&m368;
	mapper.internal_struct_size[0] = sizeof(m368);

	if (info.reset >= HARD) {
		memset(&m368, 0x00, sizeof(m368));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_368(void) {
	prg_fix_368();
	wram_fix_368();
}
void extcl_cpu_wr_mem_368(BYTE nidx, WORD address, BYTE value) {
		switch (address & 0xF1FF)
			case 0x4022: {
				m368.reg[0] = value;
				prg_fix_368();
				break;
			case 0x4122:
				m368.reg[1] = value;
				m368.irq.enable = value & 0x01;
				if (!m368.irq.enable) {
					m368.irq.counter = 0;
					nes[nidx].c.irq.high &= ~EXT_IRQ;
				}
				break;
			default:
				break;
		}
}
BYTE extcl_cpu_rd_mem_368(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	return ((address & 0xF1FF) == 0x4122 ? 0x8A | (m368.reg[1] & 0x35) : wram_rd(nidx, address));
}
BYTE extcl_save_mapper_368(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m368.reg);
	save_slot_ele(mode, slot, m368.irq.enable);
	save_slot_ele(mode, slot, m368.irq.counter);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_368(BYTE nidx) {
	if (m368.irq.enable) {
		m368.irq.counter = (m368.irq.counter + 1) & 0x0FFF;
		if (!m368.irq.counter) {
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_368(void) {
	memmap_auto_8k(0, MMCPU(0x8000), 0x01);
	memmap_auto_8k(0, MMCPU(0xA000), 0x00);
	// Value  Bank#
	// ------------
	// 0      4
	// 1      3
	// 2      5
	// 3      3
	// 4      6
	// 5      3
	// 6      7
	// 7      3
	memmap_auto_8k(0, MMCPU(0xC000), (m368.reg[0] & 0x01 ? 3 : 4 | ((m368.reg[0] & 0x06) >> 1)));
	memmap_auto_8k(0, MMCPU(0xE000), 0x08);
}
INLINE static void wram_fix_368(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000), 0x02);
}
