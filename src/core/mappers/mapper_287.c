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

void prg_swap_mmc3_287(WORD address, WORD value);
void chr_swap_mmc3_287(WORD address, WORD value);

struct _m287 {
	WORD reg;
} m287;

void map_init_287(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(287);
	EXTCL_SAVE_MAPPER(287);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m287;
	mapper.internal_struct_size[0] = sizeof(m287);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m287, 0x00, sizeof(m287));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_287;
	MMC3_chr_swap = chr_swap_mmc3_287;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_287(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(address)) {
			m287.reg = address;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_287(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m287.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_287(WORD address, WORD value) {
	if (m287.reg & 0x0008) {
		BYTE bank = ((m287.reg & 0x0007) << 2) | ((m287.reg & 0x0030) >> 4);

		value = (bank << 2) | ((address >> 13) & 0x03);
	} else {
		WORD base = (m287.reg & 0x0007) << 4;
		WORD mask = 0x0F;

		value = (base & ~mask) | (value & mask);
	}
	prg_swap_MMC3_base(address, value);
}
void chr_swap_mmc3_287(WORD address, WORD value) {
	WORD base = (m287.reg & 0x0007) << 7;
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}