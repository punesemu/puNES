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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void ks7016_6000_update(void);

struct _ks7016 {
	BYTE reg;
} ks7016;
struct _ks7016tmp {
	BYTE *prg_6000;
} ks7016tmp;

void map_init_KS7016(void) {
	EXTCL_CPU_WR_MEM(KS7016);
	EXTCL_CPU_RD_MEM(KS7016);
	EXTCL_SAVE_MAPPER(KS7016);
	mapper.internal_struct[0] = (BYTE *)&ks7016;
	mapper.internal_struct_size[0] = sizeof(ks7016);

	{
		BYTE value;

		value = 12;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);
		value = 13;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);
		value = 14;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);
		value = 15;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}

	ks7016.reg = 8;
	ks7016_6000_update();
}
void extcl_cpu_wr_mem_KS7016(WORD address, UNUSED(BYTE value)) {
	WORD mask = (address & 0x30);

	switch (address & 0xD943) {
		case 0xD943:
			if (mask == 0x30) {
				ks7016.reg = 0x08 | 0x03;
			} else {
				ks7016.reg = (address >> 2) & 0x0F;
			}
			ks7016_6000_update();
			return;
		case 0xD903:
			if (mask == 0x30) {
				ks7016.reg = 0x08 | ((address >> 2) & 0x03);
			} else {
				ks7016.reg = 0x08 | 0x03;
			}
			ks7016_6000_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_KS7016(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (ks7016tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7016(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7016.reg);

	if (mode == SAVE_SLOT_READ) {
		ks7016_6000_update();
	}

	return (EXIT_OK);
}

INLINE static void ks7016_6000_update(void) {
	WORD value;

	value = ks7016.reg;
	control_bank(info.prg.rom[0].max.banks_8k)
	ks7016tmp.prg_6000 = prg_chip_byte_pnt(0, value << 13);
}
