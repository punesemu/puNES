/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

void map_init_DREAMTECH01(void) {
	EXTCL_CPU_WR_MEM(DREAMTECH01);
	info.mapper.extend_wr = TRUE;

	//if (info.reset >= HARD) {
		map_prg_rom_8k(2, 0, 0);
		map_prg_rom_8k(2, 0, 8);
	//}
}

void extcl_cpu_wr_mem_DREAMTECH01(WORD address, BYTE value) {
	if (address != 0x5020) {
		return;
	}

	control_bank_with_AND(0x07, info.prg.rom[0].max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}
