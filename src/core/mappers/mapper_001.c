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
#include "mem_map.h"

void prg_swap_mmc1_001(WORD address, WORD value);
void chr_swap_mmc1_001(WORD address, WORD value);
void wram_fix_mmc1_001(void);
void mirroring_fix_mmc1_001(void);

INLINE static void tmp_fix_001(BYTE max, BYTE index, const BYTE *ds);

struct _m001tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m001tmp;

void map_init_001(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_CPU_RD_RAM(001);
	EXTCL_SAVE_MAPPER(MMC1);
	mapper.internal_struct[0] = (BYTE *)&mmc1;
	mapper.internal_struct_size[0] = sizeof(mmc1);

	init_MMC1((info.mapper.id == 155) || (info.mapper.submapper == 3) ? MMC1A : MMC1B);
	MMC1_prg_swap = prg_swap_mmc1_001;
	MMC1_chr_swap = chr_swap_mmc1_001;
	MMC1_wram_fix = wram_fix_mmc1_001;
	MMC1_mirroring_fix = mirroring_fix_mmc1_001;

	if (info.reset == RESET) {
		if (m001tmp.ds_used) {
			m001tmp.index = (m001tmp.index + 1) % m001tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0xAF8F7059) { // NTF2 System Cart (U) [!].nes
			static BYTE ds[] = { 0x07, 0xFD, 0x03, 0xFE };

			tmp_fix_001(LENGTH(ds), 0, &ds[0]);
		}
	}

	if (info.mapper.submapper == DEFAULT) {
		if (((info.prg.rom.banks_8k == 16) || (info.prg.rom.banks_8k == 32) || (info.prg.rom.banks_8k == 64))
			&& (info.chr.rom.banks_8k <= 1)
			&& ((info.prg.ram.banks_8k_plus == 4) || (info.prg.ram.bat.banks == 4))) {
			info.mapper.submapper = SXROM;
		} else if (info.prg.rom.banks_8k <= 32) {
			if (info.chr.rom.banks_8k <= 1) {
				info.mapper.submapper = SNROM;
			}
		} else {
			info.mapper.submapper = SUROM;
		}
	}

	switch (info.mapper.submapper) {
		case SNROM:
			// SUROM usa 8k di PRG Ram
			info.prg.ram.banks_8k_plus = 1;
			break;
		case SOROM:
			// SOROM usa 16k di PRG Ram
			info.prg.ram.banks_8k_plus = 2;
			break;
		case SXROM:
			// SXROM usa 32k di PRG Ram
			info.prg.ram.banks_8k_plus = 4;
			break;
		default:
			break;
	}
}
BYTE extcl_cpu_rd_ram_001(WORD address, BYTE openbus) {
	if (m001tmp.ds_used && (address >= 0x1000) && (address <= 0x1FFF)) {
		return (m001tmp.dipswitch[m001tmp.index]);
	}
	return (openbus);
}

void prg_swap_mmc1_001(WORD address, WORD value) {
	value = info.mapper.submapper == SEROM
		? (address >> 14) & 0x01
		: (chr_bank_MMC1(0) & 0x10) | (value & 0x0F);
	prg_swap_MMC1_base(address, value);
}
void chr_swap_mmc1_001(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (value & 0x1F));
}
void wram_fix_mmc1_001(void) {
	WORD bank = chr_bank_MMC1(0);

	if (wram_size() == (16 * 1024)) {
		bank = chr_size() ? (~bank & 0x10) >> 4 : (~bank & 0x08) >> 3;
	} else if (wram_size() == (32 * 1024)) {
		bank = (bank & 0x0C) >> 3;
	} else if (mmc1tmp.type == MMC1A) {
		bank = (bank & 0x08) >> 3;
	}
	MMC1_wram_swap(0x6000, bank);
}
void mirroring_fix_mmc1_001(void) {
	if (mapper.mirroring == MIRRORING_FOURSCR) {
		return;
	}
	mirroring_fix_MMC1_base();
}

INLINE static void tmp_fix_001(BYTE max, BYTE index, const BYTE *ds) {
	m001tmp.ds_used = TRUE;
	m001tmp.max = max;
	m001tmp.index = index;
	m001tmp.dipswitch = ds;
}
