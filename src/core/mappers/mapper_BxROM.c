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

void map_init_BxROM(void) {
	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	switch (info.mapper.submapper) {
		case BXROMBC:
			EXTCL_CPU_WR_MEM(BxROM);
			break;
		case AVENINA001:
			info.mapper.extend_wr = TRUE;
			EXTCL_CPU_WR_MEM(AveNina001);
			break;
		default:
		case BXROMUNL:
			EXTCL_CPU_WR_MEM(BxROM_UNL);
			break;
	}
}

void extcl_cpu_wr_mem_BxROM(WORD address, BYTE value) {
	/* bus conflict */
	value &= prg_rom_rd(address);

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_BxROM_UNL(UNUSED(WORD address), BYTE value) {
	control_bank_with_AND(0x3F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_AveNina001(WORD address, BYTE value) {
	DBWORD bank;

	if ((address >= 0x8000) &&  (info.prg.rom.max.banks_32k > 1)) {
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		return;
	}

	switch (address) {
		case 0x7FFD:
			control_bank_with_AND(0x01, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			break;
		case 0x7FFE:
			control_bank_with_AND(0x1F, info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[0] = chr_pnt(bank);
			chr.bank_1k[1] = chr_pnt(bank | 0x0400);
			chr.bank_1k[2] = chr_pnt(bank | 0x0800);
			chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
			break;
		case 0x7FFF:
			control_bank_with_AND(0x1F, info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[4] = chr_pnt(bank);
			chr.bank_1k[5] = chr_pnt(bank | 0x0400);
			chr.bank_1k[6] = chr_pnt(bank | 0x0800);
			chr.bank_1k[7] = chr_pnt(bank | 0x0C00);
			break;
	}
}
