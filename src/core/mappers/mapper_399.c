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
#include "irqA12.h"
#include "save_slot.h"

void prg_fix_399(void);
void chr_fix_399(void);

struct _m399 {
	BYTE reg[4];
} m399;

void map_init_399(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(399);
	EXTCL_SAVE_MAPPER(399);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m399;
	mapper.internal_struct_size[0] = sizeof(m399);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();
	MMC3_prg_fix = prg_fix_399;
	MMC3_chr_fix = chr_fix_399;

	m399.reg[0] = m399.reg[2] = 0;
	m399.reg[1] = m399.reg[3] = 1;

	if (mapper.write_vram) {
		info.chr.rom.banks_8k = 4;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_399(WORD address, BYTE value) {
	if ((address >= 0x8000) && (address <= 0x9FFF)) {
		m399.reg[(!(address & 0x0001) << 1) | (value >> 7)] = value;
		MMC3_prg_fix();
		MMC3_chr_fix();
		return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_399(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m399.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_fix_399(void) {
	WORD bank = 0;

	bank = 0;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = m399.reg[0];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = m399.reg[1];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
void chr_fix_399(void) {
	DBWORD bank;

	bank = m399.reg[2];
	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x400);
	chr.bank_1k[2] = chr_pnt(bank | 0x800);
	chr.bank_1k[3] = chr_pnt(bank | 0xC00);

	bank = m399.reg[3];
	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x400);
	chr.bank_1k[6] = chr_pnt(bank | 0x800);
	chr.bank_1k[7] = chr_pnt(bank | 0xC00);
}
