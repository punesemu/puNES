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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"

void map_init_K3036(void) {
	EXTCL_CPU_WR_MEM(K3036);

	extcl_cpu_wr_mem_K3036(0x8000, 0xFF);
}
void extcl_cpu_wr_mem_K3036(WORD address, UNUSED(BYTE value)) {
	BYTE outer = address & 0x1F;

	value = outer;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	value = outer | (address & 0x0020 ? 0 : 7);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();

	if ((address & 0x0025) == 0x0025) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
