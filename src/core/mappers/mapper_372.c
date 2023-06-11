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

void prg_swap_mmc3_372(WORD address, WORD value);
void chr_swap_mmc3_372(WORD address, WORD value);

struct _m372 {
	BYTE index;
	BYTE reg[4];
} m372;

void map_init_372(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(372);
	EXTCL_SAVE_MAPPER(372);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m372;
	mapper.internal_struct_size[0] = sizeof(m372);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m372, 0x00, sizeof(m372));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_372;
	MMC3_chr_swap = chr_swap_mmc3_372;

	m372.reg[2] = 0x0F;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_372(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(MMCPU(address))) {
			m372.reg[m372.index] = value;
			m372.index = (m372.index + 1) & 0x03;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_372(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m372.index);
	save_slot_ele(mode, slot, m372.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_372(WORD address, WORD value) {
	WORD base = m372.reg[1] | ((m372.reg[2] & 0xC0) << 2);
	WORD mask = ~m372.reg[3] & 0x3F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_372(WORD address, WORD value) {
	if ((m372.reg[2] & 0x20) && vram_size()) {
		memmap_vram_1k(MMPPU(address), address >> 10);
	} else {
		WORD base = m372.reg[0] | (m372.reg[2] & 0xF0) << 4;
		WORD mask = 0xFF >> (~m372.reg[2] & 0x0F);

		chr_swap_MMC3_base(address, (base | (value & mask)));
	}
}
