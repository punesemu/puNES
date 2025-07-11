/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include "info.h"

void prg_swap_mmc3_325(WORD address, WORD value);
void chr_swap_mmc3_325(WORD address, WORD value);

void map_init_325(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(325);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_325;
	MMC3_chr_swap = chr_swap_mmc3_325;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_325(BYTE nidx, WORD address, BYTE value) {
	if (address < 0xC000) {
		address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01);
	} else {
		address = (address & 0xFFFE) | ((address >> 3) & 0x01);
	}
	extcl_cpu_wr_mem_MMC3(nidx, address, value);
}

void prg_swap_mmc3_325(WORD address, WORD value) {
	value = (value & 0x03) | ((value >> 1) & 0x04) | ((value << 1) & 0x08);
	prg_swap_MMC3_base(address, value);
}
void chr_swap_mmc3_325(WORD address, WORD value) {
	value = (value & 0xDD) | ((value >> 4) & 0x02) | ((value << 4) & 0x20) | (address >> 10);
	chr_swap_MMC3_base(address, value);
}
