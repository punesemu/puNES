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

INLINE static void prg_fix_100(void);
INLINE static void chr_fix_100(void);

struct _m100 {
	BYTE reg;
	BYTE prg[4];
	BYTE chr[8];
} m100;

void map_init_100(void) {
	EXTCL_AFTER_MAPPER_INIT(100);
	EXTCL_CPU_INIT_PC(100);
	EXTCL_CPU_WR_MEM(100);
	EXTCL_SAVE_MAPPER(100);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m100;
	mapper.internal_struct_size[0] = sizeof(m100);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m100, 0x00, sizeof(m100));

	init_MMC3();

	m100.reg = 0;
	m100.prg[0] = 0;
	m100.prg[1] = 1;
	m100.prg[2] = 0xFE;
	m100.prg[3] = 0xFF;
	m100.chr[0] = 0;
	m100.chr[1] = 1;
	m100.chr[2] = 2;
	m100.chr[3] = 3;
	m100.chr[4] = 4;
	m100.chr[5] = 5;
	m100.chr[6] = 6;
	m100.chr[7] = 7;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_100(void) {
	extcl_after_mapper_init_MMC3();
	prg_fix_100();
	chr_fix_100();
}
void extcl_cpu_init_pc_100(void) {
	if (info.reset >= HARD) {
		// trainer
		if (miscrom.trainer.in_use && wram_size()) {
			if (miscrom_byte(0) == 0x4C) {
				cpu.PC.w = 0x7000;
			}
		}
	}
}
void extcl_cpu_wr_mem_100(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000:
			m100.reg = value;
			return;
		case 0x8001: {
			switch (m100.reg) {
				case 0x00:
					m100.chr[0] = value & 0xFE;
					m100.chr[1] = value | 0x01;
					break;
				case 0x01:
					m100.chr[2] = value & 0xFE;
					m100.chr[3] = value | 0x01;
					break;
				case 0x02:
					m100.chr[4] = value;
					break;
				case 0x03:
					m100.chr[5] = value;
					break;
				case 0x04:
					m100.chr[6] = value;
					break;
				case 0x05:
					m100.chr[7] = value;
					break;
				case 0x06:
					m100.prg[0] = value;
					break;
				case 0x07:
					m100.prg[1] = value;
					break;
				case 0x46:
					m100.prg[2] = value;
					break;
				case 0x47:
					m100.prg[1] = value;
					break;
				case 0x80:
					m100.chr[4] = value & 0xFE;
					m100.chr[5] = value | 0x01;
					break;
				case 0x81:
					m100.chr[6] = value & 0xFE;
					m100.chr[7] = value | 0x01;
					break;
				case 0x82:
					m100.chr[0] = value;
					break;
				case 0x83:
					m100.chr[1] = value;
					break;
				case 0x84:
					m100.chr[2] = value;
					break;
				case 0x85:
					m100.chr[3] = value;
					break;
			}
			prg_fix_100();
			chr_fix_100();
			return;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_100(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m100.reg);
	save_slot_ele(mode, slot, m100.prg);
	save_slot_ele(mode, slot, m100.chr);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

INLINE static void prg_fix_100(void) {
	memmap_auto_8k(MMCPU(0x8000), m100.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m100.prg[1]);
	memmap_auto_8k(MMCPU(0xC000), m100.prg[2]);
	memmap_auto_8k(MMCPU(0xE000), m100.prg[3]);
}
INLINE static void chr_fix_100(void) {
	memmap_auto_1k(MMPPU(0x0000), m100.chr[0]);
	memmap_auto_1k(MMPPU(0x0400), m100.chr[1]);
	memmap_auto_1k(MMPPU(0x0800), m100.chr[2]);
	memmap_auto_1k(MMPPU(0x0C00), m100.chr[3]);
	memmap_auto_1k(MMPPU(0x1000), m100.chr[4]);
	memmap_auto_1k(MMPPU(0x1400), m100.chr[5]);
	memmap_auto_1k(MMPPU(0x1800), m100.chr[6]);
	memmap_auto_1k(MMPPU(0x1C00), m100.chr[7]);
}
