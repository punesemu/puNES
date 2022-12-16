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

#include "mem_map.h"
#include "info.h"
#include "mappers.h"

void map_init_541(void) {
	EXTCL_CPU_WR_MEM(541);

	map_prg_rom_8k(4, 0, 0);
}
void extcl_cpu_wr_mem_541(WORD address, BYTE value) {
	if (address < 0xC000) {
		return;
	}

	if (address & 0x0002) {
		value = (address & 0xFC) >> 2;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = (address & 0xF8) >> 3;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();

	if (address & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
