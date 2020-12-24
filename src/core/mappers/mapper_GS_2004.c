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
#include "mem_map.h"
#include "info.h"

struct _gs2004tmp {
	BYTE *prg_6000;
} gs2004tmp;

void map_init_GS_2004(void) {
	EXTCL_CPU_WR_MEM(GS_2004);
	EXTCL_CPU_RD_MEM(GS_2004);

	{
		BYTE value = 0xFF;

		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	gs2004tmp.prg_6000 = prg_chip_byte_pnt(1, 0);
}
void extcl_cpu_wr_mem_GS_2004(UNUSED(WORD address), BYTE value) {
	control_bank(info.prg.rom[0].max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_GS_2004(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (gs2004tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
