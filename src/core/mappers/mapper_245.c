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
#include "irqA12.h"

void prg_swap_mmc3_245(WORD address, WORD value);

void map_init_245(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(245);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_245;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_245(BYTE nidx, WORD address, BYTE value) {
	if ((address & 0xE001) == 0x8001) {
		const WORD reg0 = mmc3.reg[0];

		extcl_cpu_wr_mem_MMC3(nidx, address, value);
		if (mmc3.reg[0] != reg0) {
			MMC3_prg_fix();
		}
		return;
	}
	extcl_cpu_wr_mem_MMC3(nidx, address, value);
}

void prg_swap_mmc3_245(WORD address, WORD value) {
	WORD base = (mmc3.reg[0] & 0x02) << 5;
	WORD mask = 0x3F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
