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
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"


void prg_swap_1(WORD address, WORD value);
void chr_swap_1(WORD address, WORD value);
void wram_fix_1(void);
void mirroring_fix_1(void);

INLINE static void tmp_fix_1(BYTE max, BYTE index, const BYTE *ds);

struct _m1tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m1tmp;

void map_init_1(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_CPU_RD_RAM(1);
	EXTCL_SAVE_MAPPER(MMC1);
	mapper.internal_struct[0] = (BYTE *)&mmc1;
	mapper.internal_struct_size[0] = sizeof(mmc1);

	init_MMC1((info.mapper.id == 155) || (info.mapper.submapper == 3) ? MMC1A : MMC1B);
	MMC1_prg_swap = prg_swap_1;
	MMC1_chr_swap = chr_swap_1;
	MMC1_wram_fix = wram_fix_1;
	MMC1_mirroring_fix = mirroring_fix_1;

	if (info.reset == RESET) {
		if (m1tmp.ds_used) {
			m1tmp.index = (m1tmp.index + 1) % m1tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0xAF8F7059) { // NTF2 System Cart (U) [!].nes
			static BYTE ds[] = { 0x07, 0xFD, 0x03, 0xFE };

			tmp_fix_1(LENGTH(ds), 0, &ds[0]);
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
BYTE extcl_cpu_rd_ram_1(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (m1tmp.ds_used && (address >= 0x1000) && (address <= 0x1FFF)) {
		return (m1tmp.dipswitch[m1tmp.index]);
	}
	return (openbus);
}

void prg_swap_1(WORD address, WORD value) {
	value = info.mapper.submapper == SEROM
		? (address >> 14) & 0x01
		: (chr_bank_MMC1(0) & 0x10) | (value & 0x0F);
	prg_swap_MMC1(address, value);
}
void chr_swap_1(WORD address, WORD value) {
	chr_swap_MMC1(address, (value & 0x1F));
}
void wram_fix_1(void) {
	WORD bank = chr_bank_MMC1(0);

	if (prg_ram_plus_size() == (16 * 1024)) {
		bank = chr_size() ? (~bank & 0x10) >> 4 : (~bank & 0x08) >> 3;
	} else if (prg_ram_plus_size() == (32 * 1024)) {
		bank = (bank & 0x0C) >> 3;
	} else if (mmc1tmp.type == MMC1A) {
		bank = (bank & 0x08) >> 3;
	}
	MMC1_wram_swap(bank);
}
void mirroring_fix_1(void) {
	if (mapper.mirroring == MIRRORING_FOURSCR) {
		return;
	}
	mirroring_fix_MMC1();
}

INLINE static void tmp_fix_1(BYTE max, BYTE index, const BYTE *ds) {
	m1tmp.ds_used = TRUE;
	m1tmp.max = max;
	m1tmp.index = index;
	m1tmp.dipswitch = ds;
}
