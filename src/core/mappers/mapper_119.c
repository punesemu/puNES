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

void chr_swap_119(WORD address, WORD value);

void map_init_119(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(MMC3);
	EXTCL_SAVE_MAPPER(119);
	EXTCL_WR_CHR(119);
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
	MMC3_chr_swap = chr_swap_119;

	if (info.format != NES_2_0) {
		info.chr.ram.banks_8k_plus = 1;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
BYTE extcl_save_mapper_119(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix(mmc3.bank_to_update);
	}

	return (EXIT_OK);
}
void extcl_wr_chr_119(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (map_chr_ram_slot_in_range(slot)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void chr_swap_119(WORD address, WORD value) {
	const BYTE slot = address >> 10;

	if (value & 0x40) {
		control_bank_with_AND(0x3F, info.chr.ram.max.banks_1k)
		chr.bank_1k[slot] = &chr.extra.data[value << 10];
	} else {
		control_bank_with_AND(0x3F, info.chr.rom.max.banks_1k)
		chr.bank_1k[slot] = chr_pnt(value << 10);
	}
}

