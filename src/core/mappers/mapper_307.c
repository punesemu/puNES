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
#include "save_slot.h"

void prg_fix_n118_307(void);
void chr_fix_n118_307(void);
void chr_swap_n118_307(WORD address, WORD value);

INLINE static void wram_fix_307(void);

struct _m307tmp {
	BYTE *prg_B000;
} m307tmp;

void map_init_307(void) {
	EXTCL_AFTER_MAPPER_INIT(307);
	EXTCL_CPU_WR_MEM(307);
	EXTCL_CPU_RD_MEM(307);
	EXTCL_SAVE_MAPPER(307);
	mapper.internal_struct[0] = (BYTE *)&n118;
	mapper.internal_struct_size[0] = sizeof(n118);

	init_N118();
	N118_prg_fix = prg_fix_n118_307;
	N118_chr_fix = chr_fix_n118_307;
	N118_chr_swap = chr_swap_n118_307;

//	if (!wram.size) {
//		wram_set_ram_size(0x2000);
//	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_307(void) {
	extcl_after_mapper_init_N118();
	wram_fix_307();
}
void extcl_cpu_wr_mem_307(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0xA000:
			return;
		case 0xB000:
			m307tmp.prg_B000[address & 0x0FFF] = value;
			return;
		default:
			extcl_cpu_wr_mem_N118(address, value);
			return;
	}
}
BYTE extcl_cpu_rd_mem_307(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0xA000:
			return (prg.rom_8k[1][address & 0x0FFF]);
		case 0xB000:
			return (m307tmp.prg_B000[address & 0x0FFF]);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_307(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_N118(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		N118_prg_fix();
		wram_fix_307();
	}

	return (EXIT_OK);
}

void prg_fix_n118_307(void) {
	WORD bank = 0;

	// 0x8000 - 0x9000
	bank = n118.reg[6];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);
	prg.rom_8k[0] = prg_pnt(mapper.rom_map_to[0] << 13);

	// 0xA000
	bank = 0x1C;
	_control_bank(bank, info.prg.rom.max.banks_4k)
	prg.rom_8k[1] = prg_pnt(bank << 12);

	// 0xC000 - 0xD000
	bank = n118.reg[7];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);
	prg.rom_8k[2] = prg_pnt(mapper.rom_map_to[2] << 13);
}
void chr_fix_n118_307(void) {
	chr_fix_N118_base();
	map_nmt_1k(0, (n118.reg[2] & 0x01));
	map_nmt_1k(1, (n118.reg[4] & 0x01));
	map_nmt_1k(2, (n118.reg[3] & 0x01));
	map_nmt_1k(3, (n118.reg[5] & 0x01));
}
void chr_swap_n118_307(WORD address, WORD value) {
	value = (address >> 10) & 0x07;
	chr_swap_N118_base(address, value);
}

INLINE static void wram_fix_307(void) {
	wram_map_auto_4k(0x6000, 0);
	wram_map_prg_rom_4k(0x7000, 0x0F);

	// 0xB000
	m307tmp.prg_B000 = &wram.data[1 << 12];
}
