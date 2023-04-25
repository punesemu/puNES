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

void prg_swap_197(WORD address, WORD value);
void chr_fix_197(void);

struct _m197 {
	BYTE reg;
} m197;

void map_init_197(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(197);
	EXTCL_SAVE_MAPPER(197);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m197;
	mapper.internal_struct_size[0] = sizeof(m197);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m197, 0x00, sizeof(m197));

	init_MMC3();
	MMC3_prg_swap = prg_swap_197;
	MMC3_chr_fix = chr_fix_197;

	if (info.mapper.submapper == 3) {
		info.mapper.extend_wr = TRUE;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_197(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			m197.reg = value;
			MMC3_prg_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		if ((address & 0xE001) == 0x8001) {
			switch (mmc3.bank_to_update & 0x07) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					mmc3.reg[mmc3.bank_to_update & 0x07] = value;
					MMC3_chr_fix();
					return;
				default:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_197(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m197.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_197(WORD address, WORD value) {
	WORD base = info.mapper.submapper == 3 ? (m197.reg & 0x01) << 4 : 0;
	WORD mask = info.mapper.submapper == 3 ? 0x1F >> ((m197.reg & 0x08) >> 3) : 0x3F;

	value = base | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_fix_197(void) {
	WORD slot[4];
	DBWORD bank = 0;

	switch (info.mapper.submapper) {
		default:
		case 0:
		case 3:
			slot[0] = mmc3.reg[0] & (~1);
			slot[1] = mmc3.reg[0] | 1;
			slot[2] = mmc3.reg[2];
			slot[3] = mmc3.reg[3];
			break;
		case 1:
			slot[0] = mmc3.reg[1] & (~1);
			slot[1] = mmc3.reg[1] | 1;
			slot[2] = mmc3.reg[4];
			slot[3] = mmc3.reg[5];
			break;
		case 2:
			slot[0] = mmc3.reg[0] & (~1);
			slot[1] = mmc3.reg[1] | 1;
			slot[2] = mmc3.reg[2];
			slot[3] = mmc3.reg[5];
			break;
	}

	bank = slot[0];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);

	bank = slot[1];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[2] = chr_pnt(bank);
	chr.bank_1k[3] = chr_pnt(bank | 0x0400);

	bank = slot[2];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);

	bank = slot[3];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[6] = chr_pnt(bank);
	chr.bank_1k[7] = chr_pnt(bank | 0x0400);
}
