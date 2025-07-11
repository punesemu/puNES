/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

void prg_swap_mmc3_196(WORD address, WORD value);

struct _m196 {
	BYTE reg[2];
} m196;

void map_init_196(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(196);
	EXTCL_CPU_RD_MEM(196);
	EXTCL_SAVE_MAPPER(196);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m196, sizeof(m196));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m196, 0x00, sizeof(m196));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_196;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_196(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		m196.reg[0] = 1;
		m196.reg[1] = value | (value >> 4);
		MMC3_prg_fix();
		return;
	} else if (address >= 0x8000) {
		if (address >= 0xC000) {
			address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01);
		} else {
			address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01) | ((address >> 1) & 0x01);
		}
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_196(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (dipswitch.value);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_196(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m196.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_196(WORD address, WORD value) {
	const BYTE slot = (address >> 13) & 0x03;

	if (m196.reg[0]) {
		value = (m196.reg[1] << 2) | slot;
	}
	prg_swap_MMC3_base(address, value);
}
