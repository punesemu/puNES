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

void prg_swap_mmc3_052(WORD address, WORD value);
void chr_swap_mmc3_052(WORD address, WORD value);

struct _m052 {
	BYTE reg;
} m052;
struct _m052tmp {
	BYTE mario7in1;
} m052tmp;

void map_init_052(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(052);
	EXTCL_SAVE_MAPPER(052);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m052, sizeof(m052));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m052, 0x00, sizeof(m052));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_052;
	MMC3_chr_swap = chr_swap_mmc3_052;

	if (info.crc32.prg == 0xC021A8B9) {
		m052tmp.mario7in1 = TRUE;
	} else {
		m052tmp.mario7in1 = FALSE;
	}

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_052(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m052.reg & 0x80) && memmap_adr_is_readable(nidx, MMCPU(address))) {
			m052.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_052(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m052.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_052(WORD address, WORD value) {
	WORD base = (m052.reg & 0x07) << 4;
	WORD mask = 0x1F >> ((m052.reg & 0x08) >> 3);

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_052(WORD address, WORD value) {
	WORD base = m052tmp.mario7in1
		? ((m052.reg & 0x20) << 4) | ((m052.reg & 0x04) << 6) | (m052.reg & 0x40 ? (m052.reg & 0x10) << 3 : 0x00)
		: ((m052.reg & 0x04) << 7) | ((m052.reg & 0x30) << 3);
	WORD mask = 0xFF >> ((m052.reg & 0x40) >> 6);

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
