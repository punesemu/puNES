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
#include "save_slot.h"

void map_init_UNIF43272(void) {
	EXTCL_CPU_WR_MEM(UNIF43272);
	EXTCL_CPU_RD_MEM(UNIF43272);
	EXTCL_SAVE_MAPPER(UNIF43272);
	mapper.internal_struct[0] = (BYTE *) &unif43272;
	mapper.internal_struct_size[0] = sizeof(unif43272);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_UNIF43272(0x0081, 0);
	} else if (info.reset == RESET) {
		extcl_cpu_wr_mem_UNIF43272(0x0000, 0);
	}

	mirroring_V();

	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_UNIF43272(WORD address, BYTE value) {
	unif43272.address = address;

	if ((address & 0x0081) == 0x0081) {
		value = (address & 0x38) >> 3;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_UNIF43272(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address < 0x6000) {
		return (openbus);
	}
	if (unif43272.address & 0x0400) {
		address &= 0x00FE;
		return(prg_rom_rd(address));
	}
	return (openbus);
}
BYTE extcl_save_mapper_UNIF43272(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unif43272.address);

	return (EXIT_OK);

}
