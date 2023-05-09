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
#include "save_slot.h"

void chr_swap_mmc3_199(WORD address, WORD value);
void wram_fix_mmc3_199(void);

void map_init_199(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(MMC3);
	EXTCL_SAVE_MAPPER(199);
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
	MMC3_chr_swap = chr_swap_mmc3_199;
	MMC3_wram_fix = wram_fix_mmc3_199;

//	if (prg_wram_size() < (12 * 1024)) {
//		wram_set_ram_size(4 * 1024);
//		wram_set_nvram_size(8 * 1024);
//	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
BYTE extcl_save_mapper_199(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_wram_fix();
	}

	return (EXIT_OK);
}

void chr_swap_mmc3_199(WORD address, UNUSED(WORD value)) {
	chr_swap_MMC3_base(address, (address >> 10));
}
void wram_fix_mmc3_199(void) {
	wram_map_auto_4k(0x5000, 2);
	wram_fix_MMC3_base();
}
