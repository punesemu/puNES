/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

static void INLINE whirlwind_6000_update(void);

BYTE *whirlwind_prg_6000;

void map_init_Whirlwind(void) {
	EXTCL_CPU_WR_MEM(Whirlwind);
	EXTCL_CPU_RD_MEM(Whirlwind);
	EXTCL_SAVE_MAPPER(Whirlwind);
	mapper.internal_struct[0] = (BYTE *) &whirlwind;
	mapper.internal_struct_size[0] = sizeof(whirlwind);

	info.prg.ram.banks_8k_plus = FALSE;

	if (info.reset >= HARD) {
		memset(&whirlwind, 0x00, sizeof(whirlwind));

		map_prg_rom_8k(4, 0, info.prg.rom[0].max.banks_32k);
	}

	whirlwind_6000_update();
}
void extcl_cpu_wr_mem_Whirlwind(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0xF000:
			whirlwind.reg = value;
			whirlwind_6000_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_Whirlwind(WORD address, BYTE openbus, BYTE before) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			return (whirlwind_prg_6000[address & 0x1FFF]);
	}
	return (openbus);

}
BYTE extcl_save_mapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, whirlwind.reg);

    if (mode == SAVE_SLOT_READ) {
    	if (save_slot.version < 15) {
    		whirlwind.reg >>= 13;
    	}
    	whirlwind_6000_update();
	}

	return (EXIT_OK);
}

static void INLINE whirlwind_6000_update(void) {
	WORD value;

	value = whirlwind.reg;
	control_bank(info.prg.rom[0].max.banks_8k)
	whirlwind_prg_6000 = prg_chip_byte_pnt(0, value << 13);
}
