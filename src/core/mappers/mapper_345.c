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

void prg_swap_mmc3_345(WORD address, WORD value);
void mirroring_fix_mmc3_345(void);

struct _m345 {
	BYTE reg;
} m345;

void map_init_345(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(345);
	EXTCL_SAVE_MAPPER(345);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m345, sizeof(m345));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	// Ho trovato questa nota:
	// Note: Cabal leaves the APU in a bad state so that
	// after soft resetting and loading Track & Field there
	// is a constant drone pitch. This is likely a game bug.

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m345, 0x00, sizeof(m345));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_345;
	MMC3_mirroring_fix = mirroring_fix_mmc3_345;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_345(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			m345.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
			MMC3_mirroring_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_345(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m345.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_345(WORD address, WORD value) {
	WORD base = (m345.reg & 0xC0) >> 2;
	WORD mask = 0x0F;

	// AxROM mode
	if (!(m345.reg & 0x0C)) {
		 base = ((base >> 2) | (m345.reg & 0x03)) << 2;
		 value = ((address >> 13) & 0x03);
		 mask = 0x03;
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void mirroring_fix_mmc3_345(void) {
	if (m345.reg & 0x20) {
		if (m345.reg & 0x10) {
			mirroring_SCR1(0);
		} else {
			mirroring_SCR0(0);
		}
		return;
	}
	mirroring_fix_MMC3_base();
}
