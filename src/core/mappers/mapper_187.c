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

void prg_swap_mmc3_187(WORD address, WORD value);
void chr_swap_mmc3_187(WORD address, WORD value);

struct _m187 {
	BYTE reg;
} m187;

void map_init_187(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(187);
	EXTCL_CPU_RD_MEM(187);
	EXTCL_SAVE_MAPPER(187);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m187;
	mapper.internal_struct_size[0] = sizeof(m187);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	if (info.reset >= HARD) {
		memset(&m187, 0x00, sizeof(m187));
	}

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_187;
	MMC3_chr_swap = chr_swap_mmc3_187;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_187(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (!(address & 0x0001)) {
			m187.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_187(WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (openbus | 0x80);
	}
	return (openbus);
}
BYTE extcl_save_mapper_187(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m187.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_187(WORD address, WORD value) {
	const WORD slot = (address >> 13) & 0x03;
	WORD mask = 0x3F;

	if (m187.reg & 0x80) {
		value = (m187.reg & 0x1F) >> 1;
		mask = ~0;
		if (m187.reg & 0x20) {
			value = ((value & ~1) << 1) | slot;
		} else {
			value = (value << 1) | (slot & 0x01);
		}
	}
	prg_swap_MMC3_base(address, (value & mask));
}
void chr_swap_mmc3_187(WORD address, WORD value) {
	const BYTE slot = address >> 10;
	WORD base = 0;

	if ((slot & 0x04) == ((mmc3.bank_to_update & 0x80) >> 5)) {
		base = 0x100;
	}
	chr_swap_MMC3_base(address, (base | value));
}
