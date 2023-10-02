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

void prg_fix_mmc3_399(void);
void chr_fix_mmc3_399(void);

struct _m399 {
	BYTE reg[4];
} m399;

void map_init_399(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(399);
	EXTCL_SAVE_MAPPER(399);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m399;
	mapper.internal_struct_size[0] = sizeof(m399);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		m399.reg[0] = m399.reg[2] = 0;
		m399.reg[1] = m399.reg[3] = 1;
	}

	init_MMC3(info.reset);
	MMC3_prg_fix = prg_fix_mmc3_399;
	MMC3_chr_fix = chr_fix_mmc3_399;

	nes[0].irqA12.present = TRUE;
	nes[0].irqA12.delay = 1;
}
void extcl_cpu_wr_mem_399(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x8000) && (address <= 0x9FFF)) {
		m399.reg[(!(address & 0x0001) << 1) | (value >> 7)] = value;
		MMC3_prg_fix();
		MMC3_chr_fix();
		return;
	}
	extcl_cpu_wr_mem_MMC3(nidx, address, value);
}
BYTE extcl_save_mapper_399(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m399.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_fix_mmc3_399(void) {
	memmap_auto_8k(0, MMCPU(0x8000), 0);
	memmap_auto_8k(0, MMCPU(0xA000), m399.reg[0]);
	memmap_auto_8k(0, MMCPU(0xC000), m399.reg[1]);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
void chr_fix_mmc3_399(void) {
	memmap_auto_4k(0, MMPPU(0x0000), m399.reg[2]);
	memmap_auto_4k(0, MMPPU(0x1000), m399.reg[3]);
}
