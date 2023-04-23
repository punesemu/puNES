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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"

void prg_swap_325(WORD address, WORD value);
void chr_swap_325(WORD address, WORD value);

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
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();
	MMC3_prg_swap = prg_swap_325;
	MMC3_chr_swap = chr_swap_325;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_325(WORD address, BYTE value) {
	if (address < 0xC000) {
		address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01);
	} else {
		address = (address & 0xFFFE) | ((address >> 3) & 0x01);
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}

void prg_swap_325(WORD address, WORD value) {
	value = (value & 0x03) | ((value >> 1) & 0x04) | ((value << 1) & 0x08);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_325(WORD address, WORD value) {
	const BYTE slot = (address >> 10);

	value = (value & 0xDD) | ((value >> 4) & 0x02) | ((value << 4) & 0x20) | slot;
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[slot] = chr_pnt(value << 10);
}
