/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

BYTE *lh32_prg_6000;

void map_init_LH32(void) {
	EXTCL_CPU_WR_MEM(LH32);
	EXTCL_CPU_RD_MEM(LH32);
	EXTCL_SAVE_MAPPER(LH32);
	mapper.internal_struct[0] = (BYTE *) &lh32;
	mapper.internal_struct_size[0] = sizeof(lh32);

	lh32.reg = 0;

	{
		BYTE value;

		value = 0xFF - 3;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = 0xFF - 2;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = 0xFF;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);

		map_prg_rom_8k_update();
	}

	lh32_prg_6000 = prg_chip_byte_pnt(0, lh32.reg << 13);

	info.prg.ram.banks_8k_plus = 1;

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
}
void extcl_cpu_wr_mem_LH32(WORD address, BYTE value) {
	if (address == 0x6000) {
		control_bank(info.prg.rom[0].max.banks_8k)
		lh32_prg_6000 = prg_chip_byte_pnt(0, value << 13);
		lh32.reg = value;
		return;
	} else if ((address & 0xE000) == 0xC000) {
		prg.ram_plus_8k[address & 0x1FFF] = value;
		return;
	}
}
BYTE extcl_cpu_rd_mem_LH32(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (lh32_prg_6000[address & 0x1FFF]);
	} else if ((address & 0xE000) == 0xC000) {
		return (prg.ram_plus_8k[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_LH32(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, lh32.reg);

	if (mode == SAVE_SLOT_READ) {
		lh32_prg_6000 = prg_chip_byte_pnt(0, lh32.reg << 13);
	}

	return (EXIT_OK);
}
