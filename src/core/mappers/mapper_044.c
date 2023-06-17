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

void prg_swap_mmc3_044(WORD address, WORD value);
void chr_swap_mmc3_044(WORD address, WORD value);

void map_init_044(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(044);
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

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_044;
	MMC3_chr_swap = chr_swap_mmc3_044;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_044(WORD address, BYTE value) {
	extcl_cpu_wr_mem_MMC3(address, value);
	if ((address & 0xE001) == 0xA001) {
		MMC3_prg_fix();
		MMC3_chr_fix();
	}
}

void prg_swap_mmc3_044(WORD address, WORD value) {
	WORD base = (mmc3.wram_protect & 0x07) << 4;
	WORD mask = (mmc3.wram_protect & 0x07) >= 6 ? 0x1F : 0x0F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_044(WORD address, WORD value) {
	WORD base = (mmc3.wram_protect & 0x07) << 7;
	WORD mask = (mmc3.wram_protect & 0x07) >= 6 ? 0xFF : 0x7F;

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
