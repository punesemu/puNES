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

INLINE static void prg_fix_526(void);
INLINE static void chr_fix_526(void);
INLINE static void mirroring_fix_526(void);

struct _m526 {
	BYTE prg[4];
	BYTE chr[8];
	struct _m526_irq {
		WORD counter;
	} irq;
} m526;

void map_init_526(void) {
	EXTCL_AFTER_MAPPER_INIT(526);
	EXTCL_CPU_WR_MEM(526);
	EXTCL_SAVE_MAPPER(526);
	EXTCL_CPU_EVERY_CYCLE(526);
	map_internal_struct_init((BYTE *)&m526, sizeof(m526));

	if (info.reset >= HARD) {
		memset(&m526, 0x00, sizeof(m526));

		m526.prg[0] = 0xFC;
		m526.prg[1] = 0xFD;
		m526.prg[2] = 0xFE;
		m526.prg[3] = 0xFF;

		m526.chr[0] = 0;
		m526.chr[1] = 1;
		m526.chr[2] = 2;
		m526.chr[3] = 3;
		m526.chr[4] = 4;
		m526.chr[5] = 5;
		m526.chr[6] = 6;
		m526.chr[7] = 7;
	}
}
void extcl_after_mapper_init_526(void) {
	prg_fix_526();
	chr_fix_526();
	mirroring_fix_526();
}
void extcl_cpu_wr_mem_526(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xE00F) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x8004:
		case 0x8005:
		case 0x8006:
		case 0x8007:
			m526.chr[address & 0x07] = value;
			chr_fix_526();
			return;
		case 0x8008:
		case 0x8009:
		case 0x800A:
		case 0x800B:
			m526.prg[address & 0x03] = value;
			prg_fix_526();
			return;
		case 0x800D:
			m526.irq.counter = 0;
			return;
		case 0x800F:
			nes[nidx].c.irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_save_mapper_526(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m526.prg);
	save_slot_ele(mode, slot, m526.chr);
	save_slot_ele(mode, slot, m526.irq.counter);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_526(BYTE nidx) {
	if (++m526.irq.counter & 0x1000) {
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_526(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m526.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m526.prg[1]);
	memmap_auto_8k(0, MMCPU(0xC000), m526.prg[2]);
	memmap_auto_8k(0, MMCPU(0xE000), m526.prg[3]);
}
INLINE static void chr_fix_526(void) {
	memmap_auto_1k(0, MMPPU(0x0000), m526.chr[0]);
	memmap_auto_1k(0, MMPPU(0x0400), m526.chr[1]);
	memmap_auto_1k(0, MMPPU(0x0800), m526.chr[2]);
	memmap_auto_1k(0, MMPPU(0x0C00), m526.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1000), m526.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1400), m526.chr[5]);
	memmap_auto_1k(0, MMPPU(0x1800), m526.chr[6]);
	memmap_auto_1k(0, MMPPU(0x1C00), m526.chr[7]);
}
INLINE static void mirroring_fix_526(void) {
	mirroring_V(0);
}
