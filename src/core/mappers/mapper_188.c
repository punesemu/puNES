/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

void map_init_188(void) {
	EXTCL_CPU_WR_MEM(188);
	EXTCL_CPU_RD_MEM(188);

	{
		BYTE value;

		extcl_cpu_wr_mem_188(0x0000, 0x00);

		value = 7;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
}
void extcl_cpu_wr_mem_188(WORD address, BYTE value) {
	if (value) {
		if (value & 0x10) {
			value = value & 0x07;
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
		} else {
			value = 0x08 | (value & 0x07);
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
		}
	} else {
		value = 7 + (info.prg.rom[0].banks_16k >> 4);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
	}
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_188(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (3);
	}
	return (openbus);
}
