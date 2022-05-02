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

struct _faridslrom8in1 {
	BYTE reg;
} faridslrom8in1;

void map_init_FARIDSLROM8IN1(void) {
	info.mapper.submapper = FARIDSLROM;
	map_init_MMC1();

	EXTCL_CPU_WR_MEM(FARIDSLROM8IN1);
	EXTCL_SAVE_MAPPER(FARIDSLROM8IN1);

	mapper.internal_struct[0] = (BYTE *)&faridslrom8in1;
	mapper.internal_struct_size[0] = sizeof(faridslrom8in1);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&faridslrom8in1, 0x00, sizeof(faridslrom8in1));

	map_prg_rom_8k(2, 0, 0);
	map_prg_rom_8k(2, 2, 7);
}
void extcl_cpu_wr_mem_FARIDSLROM8IN1(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if ((cpu.prg_ram_wr_active == TRUE) && !(faridslrom8in1.reg & 0x08)) {
			faridslrom8in1.reg = value;
			mmc1.prg_upper = (value & 0x70) >> 1;
			mmc1.chr_upper = (value & 0x70) << 1;
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC1(address, value);
	}
}
BYTE extcl_save_mapper_FARIDSLROM8IN1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, faridslrom8in1.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}
