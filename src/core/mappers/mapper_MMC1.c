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

void (*MMC1_prg_fix)(void);
void (*MMC1_prg_swap)(WORD address, WORD value);
void (*MMC1_chr_fix)(void);
void (*MMC1_chr_swap)(WORD address, WORD value);
void (*MMC1_wram_fix)(void);
void (*MMC1_wram_swap)(WORD value);
void (*MMC1_mirroring_fix)(void);

void prg_swap_MMC1_mapper1(WORD address, WORD value);
void chr_swap_MMC1_mapper1(WORD address, WORD value);
void wram_fix_MMC1_mapper1(void);
void mirroring_fix_MMC1_mapper1(void);

INLINE static void tmp_fix_MMC1(BYTE max, BYTE index, const BYTE *ds);

_mmc1 mmc1;
struct _mmc1tmp {
	BYTE type;
	// dipswitch
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} mmc1tmp;

void map_init_MMC1(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_CPU_RD_RAM(MMC1);
	EXTCL_SAVE_MAPPER(MMC1);
	mapper.internal_struct[0] = (BYTE *)&mmc1;
	mapper.internal_struct_size[0] = sizeof(mmc1);

	init_MMC1((info.mapper.id == 155) || (info.mapper.submapper == 3) ? MMC1A : MMC1B);
	MMC1_prg_swap = prg_swap_MMC1_mapper1;
	MMC1_chr_swap = chr_swap_MMC1_mapper1;
	MMC1_wram_fix = wram_fix_MMC1_mapper1;
	MMC1_mirroring_fix = mirroring_fix_MMC1_mapper1;

	if (info.reset == RESET) {
		if (mmc1tmp.ds_used) {
			mmc1tmp.index = (mmc1tmp.index + 1) % mmc1tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0xAF8F7059) { // NTF2 System Cart (U) [!].nes
			static BYTE ds[] = { 0x07, 0xFD, 0x03, 0xFE };

			tmp_fix_MMC1(LENGTH(ds), 0, &ds[0]);
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
void extcl_after_mapper_init_MMC1(void) {
	MMC1_prg_fix();
	MMC1_chr_fix();
	MMC1_wram_fix();
	MMC1_mirroring_fix();
}
void extcl_cpu_wr_mem_MMC1(WORD address, BYTE value) {
	// se nel tick precedente e' stato fatto un reset e
	// sono in presenza di una doppia scrittura da parte
	// di un'istruzione (tipo l'INC), allora l'MMC1 non
	// la considera. Roms interessate:
	// Advanced Dungeons & Dragons - Hillsfar
	// Bill & Ted's Excellent Video Game Adventure
	// Snow Brothers
	if (mmc1.reset) {
		// azzero il flag
		mmc1.reset = FALSE;
		// esco se necessario
		if (cpu.double_wr) {
			return;
		}
	}
	// A program's reset code will reset the mapper
	// first by writing a value of $80 through $FF
	// to any address in $8000-$FFFF.
	if (value & 0x80) {
		// indico che e' stato fatto un reset
		mmc1.reset = TRUE;
		// azzero posizione e registro temporaneo
		mmc1.accumulator = mmc1.shift = 0;
		// reset shift register and write
		// Control with (Control OR $0C).
		mmc1.reg[0] |= 0x0C;
		return;
	}

	mmc1.accumulator |= ((value & 0x01) << mmc1.shift);

	if (mmc1.shift++ == 4) {
		mmc1.reg[(address >> 13) & 0x03] = mmc1.accumulator;
		mmc1.accumulator = mmc1.shift = 0;
		MMC1_prg_fix();
		MMC1_chr_fix();
		MMC1_wram_fix();
		MMC1_mirroring_fix();
	}
}
BYTE extcl_cpu_rd_ram_MMC1(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (mmc1tmp.ds_used && (address >= 0x1000) && (address <= 0x1FFF)) {
		return (mmc1tmp.dipswitch[mmc1tmp.index]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_MMC1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc1.reg);
	save_slot_ele(mode, slot, mmc1.accumulator);
	save_slot_ele(mode, slot, mmc1.shift);
	save_slot_ele(mode, slot, mmc1.reset);

	return (EXIT_OK);
}

void prg_swap_MMC1_mapper1(WORD address, WORD value) {
	value = info.mapper.submapper == SEROM
		? (address >> 14) & 0x01
		: (chr_bank_MMC1(0) & 0x10) | (value & 0x0F);
	prg_swap_MMC1(address, value);
}
void chr_swap_MMC1_mapper1(WORD address, WORD value) {
	chr_swap_MMC1(address, (value & 0x1F));
}
void wram_fix_MMC1_mapper1(void) {
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
void mirroring_fix_MMC1_mapper1(void) {
	if (mapper.mirroring == MIRRORING_FOURSCR) {
		return;
	}
	mirroring_fix_MMC1();
}

void init_MMC1(BYTE type) {
	memset(&mmc1, 0x00, sizeof(mmc1));

	mmc1.reg[0] = 0x0C;
	mmc1tmp.type = type;

	MMC1_prg_fix = prg_fix_MMC1;
	MMC1_prg_swap = prg_swap_MMC1;
	MMC1_chr_fix = chr_fix_MMC1;
	MMC1_chr_swap = chr_swap_MMC1;
	MMC1_wram_fix = wram_fix_MMC1;
	MMC1_wram_swap = wram_swap_MMC1;
	MMC1_mirroring_fix = mirroring_fix_MMC1;
}
void prg_fix_MMC1(void) {
	MMC1_prg_swap(0x8000, prg_bank_MMC1(0));
	MMC1_prg_swap(0xC000, prg_bank_MMC1(1));
}
void prg_swap_MMC1(WORD address, WORD value) {
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, ((address >> 13) & 0x02), value);
	map_prg_rom_8k_update();
}
void chr_fix_MMC1(void) {
	MMC1_chr_swap(0x0000, chr_bank_MMC1(0));
	MMC1_chr_swap(0x1000, chr_bank_MMC1(1));
}
void chr_swap_MMC1(WORD address, WORD value) {
	const BYTE slot = (address >> 10) & 0x04;
	DBWORD bank = value;

	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[slot] = chr_pnt(bank);
	chr.bank_1k[slot | 0x01] = chr_pnt(bank | 0x0400);
	chr.bank_1k[slot | 0x02] = chr_pnt(bank | 0x0800);
	chr.bank_1k[slot | 0x03] = chr_pnt(bank | 0x0C00);
}
void wram_fix_MMC1(void) {
	MMC1_wram_swap(0);
}
void wram_swap_MMC1(WORD value) {
	if (mmc1tmp.type == MMC1B) {
		cpu.prg_ram_rd_active = (mmc1.reg[3] & 0x10 ? FALSE : TRUE);
		cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
	}
	if (prg.ram_plus) {
		prg.ram_plus_8k = &prg.ram_plus[(value * 0x2000) & ((prg_ram_plus_size()) - 1)];
	}

}
void mirroring_fix_MMC1(void) {
	switch (mmc1.reg[0] & 0x03) {
		case 0x00:
			mirroring_SCR0();
			break;
		case 0x01:
			mirroring_SCR1();
			break;
		case 0x02:
			mirroring_V();
			break;
		case 0x03:
			mirroring_H();
			break;
	}
}

WORD prg_bank_MMC1(int index) {
	WORD bank = 0;

	bank = mmc1.reg[0] & 0x08
		? mmc1.reg[0] & 0x04 ? (mmc1.reg[3] | index * 0x0F) : (mmc1.reg[3] & index * 0x0F)
		: (mmc1.reg[3] & ~1) | index;
	return ((mmc1.reg[3] & 0x10) && (mmc1tmp.type == MMC1A)
		? (bank & 0x07) | (mmc1.reg[3] & 0x08)
		: bank & 0x0F);
}
WORD chr_bank_MMC1(int index) {
	return (mmc1.reg[0] & 0x10 ? mmc1.reg[1 + index] : (mmc1.reg[1] & ~1) | index);
}

INLINE static void tmp_fix_MMC1(BYTE max, BYTE index, const BYTE *ds) {
	mmc1tmp.ds_used = TRUE;
	mmc1tmp.max = max;
	mmc1tmp.index = index;
	mmc1tmp.dipswitch = ds;
}
