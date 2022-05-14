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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

struct _ac08 {
	BYTE reg;
} ac08;
struct _ac08tmp {
	BYTE *prg_6000;
} ac08tmp;

void map_init_AC08(void) {
	EXTCL_CPU_WR_MEM(AC08);
	EXTCL_CPU_RD_MEM(AC08);
	EXTCL_SAVE_MAPPER(AC08);
	mapper.internal_struct[0] = (BYTE *)&ac08;
	mapper.internal_struct_size[0] = sizeof(ac08);

	ac08.reg = 0;

	extcl_cpu_wr_mem_AC08(0x8000, 0);

	{
		BYTE value;

		value = ~1;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_AC08(WORD address, BYTE value) {
	if (address >= 0x8000) {
		if (address == 0x8001) {
			value = (value >> 1) & 0x0F;
		} else {
			value = value & 0x0F;
		}
		control_bank(info.prg.rom.max.banks_8k)
		ac08tmp.prg_6000 = prg_pnt(value << 13);
		ac08.reg = value;
		return;
	}

	if (address == 0x4025) {
		if ((value >> 3) & 0x01) {
			mirroring_H();
		} else {
			mirroring_V();
		}
		return;
	}
}
BYTE extcl_cpu_rd_mem_AC08(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (ac08tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_AC08(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ac08.reg);

	if (mode == SAVE_SLOT_READ) {
		ac08tmp.prg_6000 = prg_pnt(ac08.reg << 13);
	}

	return (EXIT_OK);
}
