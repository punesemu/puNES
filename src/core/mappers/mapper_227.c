/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

void map_init_227(void) {
	EXTCL_CPU_WR_MEM(227);

	extcl_cpu_wr_mem_227(0x8000, 0x00);
}
void extcl_cpu_wr_mem_227(WORD address, BYTE value) {
	BYTE bank = ((address >> 4) & 0x10) | ((address >> 3) & 0x0F);

	if (address & 0x0001) {
		value = bank;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		value = (bank << 1) | ((address >> 2) & 0x01);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	}

	if (!(address & 0x0080)) {
		value = ((address & 0x0200) ? 0x07 : 0x00) | ((bank << 1) & 0x38);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	if (address & 0x0002) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
