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

void prg_swap_mmc3_249(WORD address, WORD value);
void chr_swap_mmc3_249(WORD address, WORD value);

INLINE static WORD calculate_bank(WORD value, const BYTE *src, const BYTE *trg, int len);

struct _m249 {
	BYTE reg;
} m249;

void map_init_249(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(249);
	EXTCL_SAVE_MAPPER(249);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m249, sizeof(m249));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m249, 0x00, sizeof(m249));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_249;
	MMC3_chr_swap = chr_swap_mmc3_249;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_249(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m249.reg = value;
		MMC3_chr_fix();
		MMC3_prg_fix();
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_249(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m249.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_249(WORD address, WORD value) {
	static const BYTE pattern[4][4] = {
		{ 3, 4, 2, 1 },
		{ 4, 3, 1, 2 },
		{ 1, 2, 3, 4 },
		{ 2, 1, 4, 3 }
	};
	WORD bank = calculate_bank(value, (BYTE *)pattern[(m249.reg & 0x03)],
		(BYTE *)pattern[(info.mapper.id == 249 ? 0 : 2)], 4);

	prg_swap_MMC3_base(address, bank);
}
void chr_swap_mmc3_249(WORD address, WORD value) {
	static const BYTE pattern[8][6] = {
		{ 5, 2, 6, 7, 4, 3 },
		{ 4, 5, 3, 2, 7, 6 },
		{ 2, 3, 4, 5, 6, 7 },
		{ 6, 4, 2, 3, 7, 5 },
		{ 5, 3, 7, 6, 2, 4 },
		{ 4, 2, 5, 6, 7, 3 },
		{ 3, 6, 4, 5, 2, 7 },
		{ 2, 5, 6, 7, 3, 4 }
	};
	WORD bank = calculate_bank(value, (BYTE *)pattern[(m249.reg & 0x07)],
		(BYTE *)pattern[(info.mapper.id == 249 ? 0 : 2)], 6);

	chr_swap_MMC3_base(address, bank);
}

INLINE static WORD calculate_bank(WORD value, const BYTE *src, const BYTE *trg, int len) {
	WORD bank = 0;
	int bit = 0;

	for (bit = 0; bit < 8; bit++) {
		if (value & (0x01 << bit)) {
			int index = 0;

			for (index = 0; index < len; index++) {
				if (src[index] == bit) {
					break;
				}
			}
			bank |= (0x01 << (index == len ? bit : trg[index]));
		}
	}
	return (bank);
}