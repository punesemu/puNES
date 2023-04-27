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

void prg_swap_mmc3_182(WORD address, WORD value);
void chr_swap_mmc3_182(WORD address, WORD value);

void map_init_182(void) {
	map_init_114();

	MMC3_prg_swap = prg_swap_mmc3_182;
	MMC3_chr_swap = chr_swap_mmc3_182;
}

void prg_swap_mmc3_182(WORD address, WORD value) {
	if (m114.reg[0] & 0x80) {
		value = ((m114.reg[0] & 0x0E) | (m114.reg[0] & 0x20 ? (address & 0x4000) >> 14 : m114.reg[0] & 0x01)) << 1;
		value |= (address & 0x2000) >> 13;
	} else {
		WORD mask = 0x0F | ((m114.reg[1] & 0x20) >> 1);
		WORD base = ((m114.reg[1] & 0x10) << 1) | ((m114.reg[1] & 0x02) << 3);

		value = base | (value & mask);
	}
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_mmc3_182(WORD address, WORD value) {
	WORD base = ((m114.reg[1] & 0x10) << 4) | ((m114.reg[1] & 0x02) << 6);
	WORD mask = 0x7F | ((m114.reg[1] & 0x40) << 1);

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
