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

void prg_swap_mmc3_262(WORD address, WORD value);
void chr_swap_mmc3_262(WORD address, WORD value);
void mirroring_fix_mmc3_262(void);

struct _m262 {
	BYTE reg;
} m262;

void map_init_262(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(262);
	EXTCL_CPU_RD_MEM(262);
	EXTCL_SAVE_MAPPER(262);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m262, sizeof(m262));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m262, 0x00, sizeof(m262));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_262;
	MMC3_chr_swap = chr_swap_mmc3_262;
	MMC3_mirroring_fix = mirroring_fix_mmc3_262;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_262(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		if (address & 0x0100) {
			m262.reg = value;
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_262(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		if (address & 0x0100) {
			return (dipswitch.value);
		}
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_262(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m262.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_262(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_262(WORD address, WORD value) {
	const BYTE slot = (address >> 10) & 0x07;

	if ((m262.reg & 0x40) && vram_size(0)) {
		memmap_vram_1k(0, MMPPU(address), slot);
	} else {
		static const BYTE shift[] = { 3, 2, 0, 1 };
		const BYTE index = (slot & 0x06) >> 1;
		WORD base = 0;

		base = ((m262.reg >> shift[index]) & 0x01) * 0x100;
		chr_swap_MMC3_base(address, (base | value));
	}
}
void mirroring_fix_mmc3_262(void) {
	mirroring_FSCR(0);
}
