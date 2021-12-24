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

struct _edu2000 {
	BYTE reg;
	uint32_t prg_ram_address;
} edu2000;

void map_init_EDU2000(void) {
	EXTCL_CPU_WR_MEM(EDU2000);
	EXTCL_CPU_RD_MEM(EDU2000);
	EXTCL_SAVE_MAPPER(EDU2000);
	mapper.internal_struct[0] = (BYTE *)&edu2000;
	mapper.internal_struct_size[0] = sizeof(edu2000);

	memset(&edu2000, 0x00, sizeof(edu2000));

	info.prg.ram.banks_8k_plus = 4;
	info.mapper.extend_rd = TRUE;
	mirroring_SCR0();
}
void extcl_cpu_wr_mem_EDU2000(UNUSED(WORD address), BYTE value) {
	BYTE save = value;

	// 0x6000 - 0x7000
	value = (save >> 6) & 0x03;
	control_bank((info.prg.ram.banks_8k_plus - 1))
	edu2000.prg_ram_address = value << 13;
	prg.ram_plus_8k = &prg.ram_plus[edu2000.prg_ram_address];

	// 0x8000 - 0xF000
	value = save & 0x1F;
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	edu2000.reg = value;
}
BYTE extcl_cpu_rd_mem_EDU2000(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address < 0x6000) {
		return (prg.ram.data[address & 0x1FFF]);
	}

	return (openbus);
}
BYTE extcl_save_mapper_EDU2000(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, edu2000.reg);
	save_slot_ele(mode, slot, edu2000.prg_ram_address);
	if (mode == SAVE_SLOT_READ) {
		prg.ram_plus_8k = prg_chip_byte_pnt(0, edu2000.prg_ram_address);
	}

	return (EXIT_OK);
}
