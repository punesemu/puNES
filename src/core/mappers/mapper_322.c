/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

void prg_swap_mmc3_322(WORD address, WORD value);
void chr_swap_mmc3_322(WORD address, WORD value);

struct _m322 {
	WORD reg;
} m322;

void map_init_322(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(322);
	EXTCL_SAVE_MAPPER(322);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m322, sizeof(m322));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m322, 0x00, sizeof(m322));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_322;
	MMC3_chr_swap = chr_swap_mmc3_322;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_322(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			m322.reg = address;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_322(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m322.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_322(WORD address, WORD value) {
	BYTE mmc3_mode = m322.reg & 0x0020;
	BYTE mode_128k = !((m322.reg & 0x0080) && mmc3_mode);

	if (mmc3_mode) {
		// modalita' MMC3
		WORD base = (((m322.reg & 0x0040) >> 4) | ((m322.reg & 0x0018) >> 3)) << (5 - mode_128k);
		WORD mask = 0x1F >> mode_128k;

		value = (base & ~mask) | (value & mask);
	} else {
		// NROM mode
		BYTE bank = ((m322.reg & 0x0040) >> 1) | (m322.reg & 0x001f);
		BYTE mode = (m322.reg & 0x03) != 0;

		bank = ((address & 0x4000) ? bank | mode : bank & ~mode);
		value = (bank << 1) | ((address & 0x2000) >> 13);
	}
	prg_swap_MMC3_base(address, value);
}
void chr_swap_mmc3_322(WORD address, WORD value) {
	BYTE mmc3_mode = m322.reg & 0x0020;
	BYTE mode_128k = !((m322.reg & 0x0080) && mmc3_mode);
	WORD base = (((m322.reg & 0x0040) >> 4) | ((m322.reg & 0x0018) >> 3)) << (8 - mode_128k);
	WORD mask = 0xFF >> mode_128k;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
