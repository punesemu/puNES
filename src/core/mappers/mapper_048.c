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

INLINE static void prg_fix_048(void);
INLINE static void chr_fix_048(void);
INLINE static void mirroring_fix_048(void);

struct _m048 {
	BYTE prg[2];
	BYTE chr[6];
	BYTE mirroring;
} m048;

void map_init_048(void) {
	EXTCL_AFTER_MAPPER_INIT(048);
	EXTCL_CPU_WR_MEM(048);
	EXTCL_SAVE_MAPPER(048);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m048;
	mapper.internal_struct_size[0] = sizeof(m048);

	memset(&irqA12, 0x00, sizeof(irqA12));

	if (info.reset >= HARD) {
		memset(&m048, 0x00, sizeof(m048));

		m048.prg[1] = 0x01;
		m048.chr[1] = 0x01;
		m048.chr[2] = 0x04;
		m048.chr[3] = 0x05;
		m048.chr[4] = 0x06;
		m048.chr[5] = 0x07;
	}

	irqA12.present = TRUE;
	irqA12_delay = 7;
}
void extcl_after_mapper_init_048(void) {
	prg_fix_048();
	chr_fix_048();
	mirroring_fix_048();
}
void extcl_cpu_wr_mem_048(WORD address, BYTE value) {
	switch (address & 0xE003) {
		case 0x8000:
		case 0x8001:
			m048.prg[address & 0x01] = value;
			prg_fix_048();
			return;
		case 0x8002:
		case 0x8003:
			m048.chr[address & 0x01] = value;
			chr_fix_048();
			return;
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
			m048.chr[(address & 0x03) + 2] = value;
			chr_fix_048();
			return;
		case 0xC000:
			irqA12.latch = value ^ 0xFF;
			return;
		case 0xC001:
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
		case 0xC002:
			irqA12.enable = TRUE;
			return;
		case 0xC003:
			irqA12.enable = FALSE;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE000:
		case 0xE001:
		case 0xE002:
		case 0xE003:
			m048.mirroring = value;
			mirroring_fix_048();
			return;
	}
}
BYTE extcl_save_mapper_048(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m048.prg);
	save_slot_ele(mode, slot, m048.chr);
	save_slot_ele(mode, slot, m048.mirroring);

	return (EXIT_OK);
}

INLINE static void prg_fix_048(void) {
	memmap_auto_8k(MMCPU(0x8000), m048.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m048.prg[1]);
	memmap_auto_16k(MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_048(void) {
	memmap_auto_2k(MMPPU(0x0000), m048.chr[0]);
	memmap_auto_2k(MMPPU(0x0800), m048.chr[1]);
	memmap_auto_1k(MMPPU(0x1000), m048.chr[2]);
	memmap_auto_1k(MMPPU(0x1400), m048.chr[3]);
	memmap_auto_1k(MMPPU(0x1800), m048.chr[4]);
	memmap_auto_1k(MMPPU(0x1C00), m048.chr[5]);
}
INLINE static void mirroring_fix_048(void) {
	if (m048.mirroring & 0x40) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
