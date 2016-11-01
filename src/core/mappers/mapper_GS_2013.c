/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include "mem_map.h"
#include "info.h"

BYTE *gs_2013_prg_6000;

void map_init_GS_2013(void) {
	EXTCL_CPU_WR_MEM(GS_2013);
	EXTCL_CPU_RD_MEM(GS_2013);

	{
		BYTE value;

		value = 0xFF;
		control_bank(info.prg.rom[0].max.banks_8k)
		gs_2013_prg_6000 = prg_chip_byte_pnt(0, value << 13);
	}

	extcl_cpu_wr_mem_GS_2013(0x0000, 0xFF);
}
void extcl_cpu_wr_mem_GS_2013(WORD address, BYTE value) {
	BYTE chip = (value >> 3) & 0x01;

	_control_bank(chip, info.prg.max_chips)
	control_bank(info.prg.rom[chip].max.banks_32k)
	map_prg_rom_8k_chip(4, 0, value, chip);
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_GS_2013(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (gs_2013_prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
