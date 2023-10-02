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

void prg_swap_mmc3_456(WORD address, WORD value);
void chr_swap_mmc3_456(WORD address, WORD value);

struct _m456 {
	BYTE reg;
} m456;

void map_init_456(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(456);
	EXTCL_SAVE_MAPPER(456);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m456;
	mapper.internal_struct_size[0] = sizeof(m456);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m456, 0x00, sizeof(m456));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_456;
	MMC3_chr_swap = chr_swap_mmc3_456;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	nes[0].irqA12.delay = 1;
}
void extcl_cpu_wr_mem_456(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if (address & 0x0100) {
			m456.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_456(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m456.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

 void prg_swap_mmc3_456(WORD address, WORD value) {
	WORD base = m456.reg << 4;
	WORD mask = 0x0F;

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_456(WORD address, WORD value) {
	WORD base = m456.reg << 7;
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
