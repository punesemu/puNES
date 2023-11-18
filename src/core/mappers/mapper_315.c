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

void prg_swap_mmc3_315(WORD address, WORD value);
void chr_swap_mmc3_315(WORD address, WORD value);

struct _m315 {
	BYTE reg;
} m315;

void map_init_315(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(315);
	EXTCL_SAVE_MAPPER(315);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m315;
	mapper.internal_struct_size[0] = sizeof(m315);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m315, 0x00, sizeof(m315));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_315;
	MMC3_chr_swap = chr_swap_mmc3_315;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_315(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			m315.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_315(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m315.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_315(WORD address, WORD value) {
	WORD base = ((m315.reg >> 1) & 0x03) << 4;
	WORD mask = 0x0F;

	// GNROM mode
	if ((m315.reg & 0x06) == 0x06) {
		BYTE bank = (address >> 13) & 0x03;
		BYTE reg = mmc3.reg[0x06 | (bank & 0x01)];

		value = bank < 2 ? reg & 0xFD : reg | 0x02;
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_315(WORD address, WORD value) {
	WORD base = (m315.reg & 0x01) << 8;
	WORD mask = 0xFF;

	// Outer Bank Register's CHR A16 and CHR A17 are OR'd with the respective MMC3 bits.
	value = ((m315.reg & 0x08) << 3) | ((m315.reg & 0x02) << 6) | value;
	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
