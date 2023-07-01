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

INLINE static void prg_fix_311(void);
INLINE static void wram_fix_311(void);

struct _m311 {
	BYTE reg;
	struct _m311_irq {
		BYTE enabled;
		uint32_t count;
	} irq;
} m311;

void map_init_311(void) {
	EXTCL_AFTER_MAPPER_INIT(311);
	EXTCL_CPU_WR_MEM(311);
	EXTCL_CPU_RD_MEM(311);
	EXTCL_SAVE_MAPPER(311);
	EXTCL_CPU_EVERY_CYCLE(311);
	mapper.internal_struct[0] = (BYTE *)&m311;
	mapper.internal_struct_size[0] = sizeof(m311);

	if (info.reset >= HARD) {
		memset(&m311, 0x00, sizeof(m311));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_311(void) {
	prg_fix_311();
	wram_fix_311();
}
void extcl_cpu_wr_mem_311(WORD address, BYTE value) {
	switch (address) {
		case 0x4022:
			m311.reg = value & 0x01;
			prg_fix_311();
			wram_fix_311();
			return;
		case 0x4122:
			m311.irq.enabled = value & 0x01;
			m311.irq.count = 0;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_311(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4042) && (address <= 0x4055)) {
		return (0xFF);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_311(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m311.reg);
	save_slot_ele(mode, slot, m311.irq.enabled);
	save_slot_ele(mode, slot, m311.irq.count);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_311(void) {
	if (m311.irq.enabled) {
		if (m311.irq.count < 4096) {
			m311.irq.count++;
		} else {
			m311.irq.count++;
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_311(void) {
	memmap_prgrom_32k(MMCPU(0x8000), m311.reg);
}
INLINE static void wram_fix_311(void) {
	memmap_prgrom_4k(MMCPU(0x5000), 17);
	memmap_prgrom_8k(MMCPU(0x6000), 9);
}
