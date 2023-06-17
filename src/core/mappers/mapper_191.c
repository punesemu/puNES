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

void prg_swap_mmc3_191(WORD address, WORD value);
void chr_swap_mmc3_191(WORD address, WORD value);

void map_init_191(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(MMC3);
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
	MMC3_prg_swap = prg_swap_mmc3_191;
	MMC3_chr_swap = chr_swap_mmc3_191;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}

void prg_swap_mmc3_191(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_191(WORD address, WORD value) {
	if ((value & 0x80) && vram_size()) {
		memmap_vram_1k(MMPPU(address), (value & 0x01));
	} else {
		chr_swap_MMC3_base(address, value);
	}
}
