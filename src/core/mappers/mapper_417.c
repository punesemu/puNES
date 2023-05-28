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
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_417(void);
INLINE static void chr_fix_417(void);
INLINE static void mirroring_fix_417(void);

struct _m417 {
	BYTE prg[3];
	BYTE chr[8];
	BYTE mir[4];
	struct _m417_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m417;

void map_init_417(void) {
	EXTCL_AFTER_MAPPER_INIT(417);
	EXTCL_CPU_WR_MEM(417);
	EXTCL_SAVE_MAPPER(417);
	EXTCL_CPU_EVERY_CYCLE(417);
	mapper.internal_struct[0] = (BYTE *)&m417;
	mapper.internal_struct_size[0] = sizeof(m417);

	memset(&m417, 0x00, sizeof(m417));
}
void extcl_after_mapper_init_417(void) {
	prg_fix_417();
	chr_fix_417();
	mirroring_fix_417();
}
void extcl_cpu_wr_mem_417(WORD address, BYTE value) {
	switch (address & 0x8073) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
			m417.prg[address & 0x03] = value;
			prg_fix_417();
			break;
		case 0x8010:
		case 0x8011:
		case 0x8012:
		case 0x8013:
			m417.chr[address & 0x03] = value;
			chr_fix_417();
			break;
		case 0x8020:
		case 0x8021:
		case 0x8022:
		case 0x8023:
			m417.chr[0x04 | (address & 0x03)] = value;
			chr_fix_417();
			break;
		case 0x8030:
			m417.irq.enable = TRUE;
			m417.irq.counter = 0;
			break;
		case 0x8040:
			m417.irq.enable = FALSE;
			irq.high &= ~EXT_IRQ;
			break;
		case 0x8050:
		case 0x8051:
		case 0x8052:
		case 0x8053:
			m417.mir[address & 0x03] = value;
			mirroring_fix_417();
			break;
	}
}
BYTE extcl_save_mapper_417(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m417.prg);
	save_slot_ele(mode, slot, m417.chr);
	save_slot_ele(mode, slot, m417.mir);
	save_slot_ele(mode, slot, m417.irq.enable);
	save_slot_ele(mode, slot, m417.irq.counter);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_417();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_417(void) {
	if ((++m417.irq.counter & 0x400) && m417.irq.enable) {
		m417.irq.counter = 0;
		irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_417(void) {
	memmap_auto_8k(MMCPU(0x8000), m417.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m417.prg[1]);
	memmap_auto_8k(MMCPU(0xC000), m417.prg[2]);
	memmap_auto_8k(MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_417(void) {
	memmap_auto_1k(MMPPU(0x0000), m417.chr[0]);
	memmap_auto_1k(MMPPU(0x0400), m417.chr[1]);
	memmap_auto_1k(MMPPU(0x0800), m417.chr[2]);
	memmap_auto_1k(MMPPU(0x0C00), m417.chr[3]);
	memmap_auto_1k(MMPPU(0x1000), m417.chr[4]);
	memmap_auto_1k(MMPPU(0x1400), m417.chr[5]);
	memmap_auto_1k(MMPPU(0x1800), m417.chr[6]);
	memmap_auto_1k(MMPPU(0x1C00), m417.chr[7]);
}
INLINE static void mirroring_fix_417(void) {
	memmap_nmt_1k(MMPPU(0x2000), (m417.mir[0] & 0x01));
	memmap_nmt_1k(MMPPU(0x2400), (m417.mir[1] & 0x01));
	memmap_nmt_1k(MMPPU(0x2800), (m417.mir[2] & 0x01));
	memmap_nmt_1k(MMPPU(0x2C00), (m417.mir[3] & 0x01));

	memmap_nmt_1k(MMPPU(0x3000), (m417.mir[0] & 0x01));
	memmap_nmt_1k(MMPPU(0x3400), (m417.mir[1] & 0x01));
	memmap_nmt_1k(MMPPU(0x3800), (m417.mir[2] & 0x01));
	memmap_nmt_1k(MMPPU(0x3C00), (m417.mir[3] & 0x01));
}
