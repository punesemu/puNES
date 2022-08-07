/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

_n118 n118;

void map_init_N118(void) {
	EXTCL_AFTER_MAPPER_INIT(N118);
	EXTCL_CPU_WR_MEM(N118);
	EXTCL_SAVE_MAPPER(N118);
	mapper.internal_struct[0] = (BYTE*) &n118;
	mapper.internal_struct_size[0] = sizeof(n118);

	memset(&n118, 0x00, sizeof(n118));

	n118.reg[0] = 0x00;
	n118.reg[1] = 0x02;
	n118.reg[2] = 0x04;
	n118.reg[3] = 0x05;
	n118.reg[4] = 0x06;
	n118.reg[5] = 0x07;
	n118.reg[6] = 0x00;
	n118.reg[7] = 0x01;
	n118.reg[8] = 0x00;
}
void extcl_after_mapper_init_N118(void) {
	prg_fix_N118(0xFFFF, 0x00);
	chr_fix_N118(0xFFFF, 0x00);
}
void extcl_cpu_wr_mem_N118(WORD address, BYTE value) {
	if ((address >= 0x8000) && (address <= 0x9FFF)) {
		if (address & 0x0001) {
			n118.reg[n118.reg[8] & 0x07] = value;
		} else {
			n118.reg[8] = value;
		}
		extcl_after_mapper_init();
	}
}
BYTE extcl_save_mapper_N118(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, n118.reg);

	return (EXIT_OK);
}

void prg_fix_N118(WORD mmask, WORD mblock) {
	WORD bank;

	bank = mblock | (n118.reg[0x06] & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = mblock | (n118.reg[0x07] & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = mblock | (0xFE & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = mblock | (0xFF & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
void chr_fix_N118(WORD mmask, WORD mblock) {
	WORD bank;

	bank = mblock | ((n118.reg[0] & 0xFE) & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[0] = chr_pnt(bank << 10);

	bank = mblock | ((n118.reg[0] | 0x01) & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[1] = chr_pnt(bank << 10);

	bank = mblock | ((n118.reg[1] & 0xFE) & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[2] = chr_pnt(bank << 10);

	bank = mblock | ((n118.reg[1] | 0x01) & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[3] = chr_pnt(bank << 10);

	bank = mblock | (n118.reg[2] & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[4] = chr_pnt(bank << 10);

	bank = mblock | (n118.reg[3] & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[5] = chr_pnt(bank << 10);

	bank = mblock | (n118.reg[4] & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[6] = chr_pnt(bank << 10);

	bank = mblock | (n118.reg[5] & mmask);
	_control_bank(bank, info.chr.rom.max.banks_1k)
	chr.bank_1k[7] = chr_pnt(bank << 10);
}
