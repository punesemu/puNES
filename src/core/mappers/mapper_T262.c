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
#include "save_slot.h"

INLINE static void t262_update(BYTE value);

struct _t262 {
	BYTE reg[5];
} t262;

void map_init_T262(void) {
	EXTCL_CPU_WR_MEM(T262);
	EXTCL_SAVE_MAPPER(T262);
	mapper.internal_struct[0] = (BYTE *)&t262;
	mapper.internal_struct_size[0] = sizeof(t262);

	memset(&t262, 0x00, sizeof(t262));

	t262_update(0);
}
void extcl_cpu_wr_mem_T262(WORD address, BYTE value) {
	if (!t262.reg[3]) {
		t262.reg[0] = ((address & 0x0060) >> 2) | ((address & 0x0100) >> 3);
		t262.reg[1] = address & 0x80;
		t262.reg[2] = address & 0x02;
		t262.reg[3] = (address & 0x2000) >> 13;
		t262.reg[4] = address & 0x01;
	}
	t262_update(value);
}
BYTE extcl_save_mapper_T262(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, t262.reg);

	return (EXIT_OK);
}

INLINE static void t262_update(BYTE value) {
	BYTE bank = value & 0x07;

	value = t262.reg[0] | bank;
	control_bank(info.prg.rom[0].max.banks_16k)
	map_prg_rom_8k(2, 0, value);

	value = t262.reg[0] | (t262.reg[1] ? bank | t262.reg[4] : 7);
	control_bank(info.prg.rom[0].max.banks_16k)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();

	if (t262.reg[2]) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
