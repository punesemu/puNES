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
#include "save_slot.h"

void prg_swap_mmc3_049(WORD address, WORD value);
void chr_swap_mmc3_049(WORD address, WORD value);

struct _m049 {
	BYTE reg;
} m049;

void map_init_049(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(049);
	EXTCL_SAVE_MAPPER(049);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m049, sizeof(m049));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m049, 0x00, sizeof(m049));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_049;
	MMC3_chr_swap = chr_swap_mmc3_049;

	m049.reg = 0x01;

	// [UNIF] street fighter ii game 4-in-1 (unl)[p1].unf
	if (info.crc32.prg == 0x408EA235) {
		m049.reg = 0x41;
	}

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_049(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			m049.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_049(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m049.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_049(WORD address, WORD value) {
	if (m049.reg & 0x01) {
		WORD base = (m049.reg & 0xC0) >> 2;
		WORD mask = 0x0F;

		value = (base & ~mask) | (value & mask);
	} else {
		// Master Fighter III
		value = ((m049.reg & 0x30) >> 2) | ((address >> 13) & 0x03);
	}
	prg_swap_MMC3_base(address, value);
}
void chr_swap_mmc3_049(WORD address, WORD value) {
	WORD base = (m049.reg & 0xC0) << 1;
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
