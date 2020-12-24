/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

void map_init_190(void) {
	EXTCL_CPU_WR_MEM(190);

	map_prg_rom_8k(2, 2, 0);

	info.prg.ram.banks_8k_plus = 1;
	mirroring_V();
}
void extcl_cpu_wr_mem_190(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			value = value & 0x07;
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			break;
		case 0xA000: {
			BYTE base = (address & 0x0003) << 1;
			DBWORD bank;

			control_bank(info.chr.rom[0].max.banks_2k)
			bank = value << 11;
			chr.bank_1k[base] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[base | 0x01] = chr_chip_byte_pnt(0, bank | 0x0400);
			return;
		}
		case 0xC000:
			value = 0x08 | (value & 0x07);
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			break;
	}
	map_prg_rom_8k_update();
}
