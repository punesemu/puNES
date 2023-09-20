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

INLINE static void prg_fix_065(void);
INLINE static void chr_fix_065(void);
INLINE static void wram_fix_065(void);
INLINE static void mirroring_fix_065(void);

struct m065 {
	BYTE prg[2];
	BYTE chr[8];
	BYTE reg[2];
	struct _m065_irq {
		BYTE enable;
		WORD count;
		WORD reload;
		BYTE delay;
	} irq;
} m065;

void map_init_065() {
	EXTCL_AFTER_MAPPER_INIT(065);
	EXTCL_CPU_WR_MEM(065);
	EXTCL_SAVE_MAPPER(065);
	EXTCL_CPU_EVERY_CYCLE(065);
	mapper.internal_struct[0] = (BYTE *)&m065;
	mapper.internal_struct_size[0] = sizeof(m065);

	if (info.reset >= HARD) {
		memset(&m065, 0x00, sizeof(m065));

		m065.prg[0] = 0;
		m065.prg[1] = 1;

		m065.chr[0] = 0;
		m065.chr[1] = 1;
		m065.chr[2] = 2;
		m065.chr[3] = 3;
		m065.chr[4] = 4;
		m065.chr[5] = 5;
		m065.chr[6] = 6;
		m065.chr[7] = 7;
	}
}
void extcl_after_mapper_init_065(void) {
	prg_fix_065();
	chr_fix_065();
	wram_fix_065();
	mirroring_fix_065();
}
void extcl_cpu_wr_mem_065(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0xA000:
			m065.prg[(address >> 13) & 0x01] = value;
			prg_fix_065();
			return;
		case 0x9000:
			switch (address & 0x07) {
				case 0:
					m065.reg[0] = value;
					prg_fix_065();
					return;
				case 1:
					m065.reg[1] = value;
					mirroring_fix_065();
					return;
				case 3:
					m065.irq.enable = value & 0x80;
					cpudata.irq.high &= ~EXT_IRQ;
					return;
				case 4:
					m065.irq.count = m065.irq.reload;
					cpudata.irq.high &= ~EXT_IRQ;
					return;
				case 5:
					m065.irq.reload = (m065.irq.reload & 0x00FF) | (value << 8);
					return;
				case 6:
					m065.irq.reload = (m065.irq.reload & 0xFF00) | value;
					return;
				default:
					return;
			}
			return;
		case 0xB000:
			m065.chr[address & 0x07] = value;
			chr_fix_065();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_065(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m065.prg);
	save_slot_ele(mode, slot, m065.chr);
	save_slot_ele(mode, slot, m065.reg);
	save_slot_ele(mode, slot, m065.irq.count);
	save_slot_ele(mode, slot, m065.irq.delay);
	save_slot_ele(mode, slot, m065.irq.enable);
	save_slot_ele(mode, slot, m065.irq.reload);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_065(void) {
	if (m065.irq.delay && !(--m065.irq.delay)) {
		cpudata.irq.high |= EXT_IRQ;
	}
	if (m065.irq.enable && m065.irq.count && !(--m065.irq.count)) {
		m065.irq.enable = FALSE;
		m065.irq.delay = 1;
		return;
	}
}

INLINE static void prg_fix_065(void) {
	WORD swap = (m065.reg[0] & 0x80) << 7;

	memmap_auto_8k(MMCPU(0x8000 ^ swap), m065.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m065.prg[1]);
	memmap_auto_8k(MMCPU(0xC000 ^ swap), 0xFE);
	memmap_auto_8k(MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_065(void) {
	memmap_auto_1k(MMPPU(0x0000), m065.chr[0]);
	memmap_auto_1k(MMPPU(0x0400), m065.chr[1]);
	memmap_auto_1k(MMPPU(0x0800), m065.chr[2]);
	memmap_auto_1k(MMPPU(0x0C00), m065.chr[3]);
	memmap_auto_1k(MMPPU(0x1000), m065.chr[4]);
	memmap_auto_1k(MMPPU(0x1400), m065.chr[5]);
	memmap_auto_1k(MMPPU(0x1800), m065.chr[6]);
	memmap_auto_1k(MMPPU(0x1C00), m065.chr[7]);
}
INLINE static void wram_fix_065(void) {
	memmap_auto_8k(MMCPU(0x6000), 0);
}
INLINE static void mirroring_fix_065(void) {
	BYTE mirroring = m065.reg[1] >> 6;

	if (mirroring == 0x00) {
		mirroring_V();
	} else if (mirroring == 0x02) {
		mirroring_H();
	} else {
		mirroring_SCR0();
	}
}
