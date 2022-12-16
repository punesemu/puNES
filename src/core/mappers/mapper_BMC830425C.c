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
#include "save_slot.h"

struct _bmc830425C {
	BYTE reg;
} bmc830425c;

void map_init_BMC830425C(void) {
	EXTCL_CPU_WR_MEM(BMC830425C);
	EXTCL_SAVE_MAPPER(BMC830425C);
	mapper.internal_struct[0] = (BYTE *)&bmc830425c;
	mapper.internal_struct_size[0] = sizeof(bmc830425c);

	memset(&bmc830425c, 0x00, sizeof(bmc830425c));

	extcl_cpu_wr_mem_BMC830425C(0x8000, 0);
}
void extcl_cpu_wr_mem_BMC830425C(WORD address, BYTE value) {
	BYTE outer, mode;

	if ((address & 0x7FE0) == 0x70E0) {
		bmc830425c.reg = address & 0x1F;
	}

	outer = (bmc830425c.reg & 0x0F) << 3;
	mode = !(bmc830425c.reg & 0x10) << 3 | 0x07;

	value = outer | (value & mode);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	value = outer | mode;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_BMC830425C(BYTE mode, BYTE slot, UNUSED(FILE *fp)) {
	save_slot_ele(mode, slot, bmc830425c.reg);

	return (EXIT_OK);
}
