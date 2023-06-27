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

#include "mappers.h"
#include "info.h"

void prg_swap_mmc1_001(WORD address, WORD value);
void chr_swap_mmc1_001(WORD address, WORD value);
void wram_fix_mmc1_001(void);
void mirroring_fix_mmc1_001(void);

void map_init_001(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_CPU_RD_RAM(001);
	EXTCL_SAVE_MAPPER(MMC1);
	mapper.internal_struct[0] = (BYTE *)&mmc1;
	mapper.internal_struct_size[0] = sizeof(mmc1);

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}

	if (info.format != NES_2_0) {
		if (!chrrom_size()) {
			if (prgrom_size() == S512K) {
				info.mapper.submapper = 4;
			} else if (prgrom_size() == S512K) {
				info.mapper.submapper = 1;
			} else {
				info.mapper.submapper = 0;
			}
		}
	}

	init_MMC1((info.mapper.id == 155) || (info.mapper.submapper == 3) ? MMC1A : MMC1B, info.reset);
	MMC1_prg_swap = prg_swap_mmc1_001;
	MMC1_chr_swap = chr_swap_mmc1_001;
	MMC1_wram_fix = wram_fix_mmc1_001;
	MMC1_mirroring_fix = mirroring_fix_mmc1_001;
}
BYTE extcl_cpu_rd_ram_001(WORD address, UNUSED(BYTE openbus)) {
	if (dipswitch.used && (address >= 0x1000) && (address <= 0x1FFF)) {
		return (dipswitch.value);
	}
	return (ram_rd(address));
}

void prg_swap_mmc1_001(WORD address, WORD value) {
	value = info.mapper.submapper == 5
		? (address >> 14) & 0x01
		: (chr_bank_MMC1(0) & 0x10) | (value & 0x0F);
	prg_swap_MMC1_base(address, value);
}
void chr_swap_mmc1_001(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (value & 0x1F));
}
void wram_fix_mmc1_001(void) {
	WORD bank = chr_bank_MMC1(0);

	if (wram_size() == S16K) {
		bank = chrrom_size() ? (~bank & 0x10) >> 4 : (~bank & 0x08) >> 3;
	} else if (wram_size() == S32K) {
		bank = (bank & 0x0C) >> 3;
	} else if (mmc1tmp.type == MMC1A) {
		bank = (bank & 0x08) >> 3;
	}
	MMC1_wram_swap(0x6000, bank);
}
void mirroring_fix_mmc1_001(void) {
	if (info.mapper.mirroring == MIRRORING_FOURSCR) {
		return;
	}
	mirroring_fix_MMC1_base();
}
