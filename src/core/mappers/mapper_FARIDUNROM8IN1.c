/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "save_slot.h"

struct _faridunrom8in1 {
	BYTE reg;
} faridunrom8in1;

void map_init_FARIDUNROM8IN1(void) {
	EXTCL_CPU_WR_MEM(FARIDUNROM8IN1);
	EXTCL_SAVE_MAPPER(FARIDUNROM8IN1);

	mapper.internal_struct[0] = (BYTE *)&faridunrom8in1;
	mapper.internal_struct_size[0] = sizeof(faridunrom8in1);

	if (info.reset == RESET) {
		faridunrom8in1.reg &= 0x87;
	} else {
		memset(&faridunrom8in1, 0x00, sizeof(faridunrom8in1));
	}

	map_prg_rom_8k(2, 0, 0);
	map_prg_rom_8k(2, 2, 7);
}
void extcl_cpu_wr_mem_FARIDUNROM8IN1(WORD address, BYTE value) {
	/* bus conflict */
	value &= prg_rom_rd(address);

	faridunrom8in1.reg = !(faridunrom8in1.reg & 0x88) && (value & 0x80) ? value : (faridunrom8in1.reg & 0x78) | (value & 0x87);

	value = ((faridunrom8in1.reg & 0x70) >> 1) | (faridunrom8in1.reg & 0x07);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	value = value | 0x07;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_FARIDUNROM8IN1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, faridunrom8in1.reg);

	return (EXIT_OK);
}
