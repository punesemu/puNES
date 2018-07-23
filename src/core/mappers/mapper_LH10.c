/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

static void INLINE lh10_update(void);

BYTE *lh10_prg_6000;
BYTE *lh10_prg_C000;

void map_init_LH10(void) {
	EXTCL_AFTER_MAPPER_INIT(LH10);
	EXTCL_CPU_WR_MEM(LH10);
	EXTCL_CPU_RD_MEM(LH10);
	EXTCL_SAVE_MAPPER(LH10);
	mapper.internal_struct[0] = (BYTE *) &lh10;
	mapper.internal_struct_size[0] = sizeof(lh10);

	memset(&lh10, 0x00, sizeof(lh10));

	info.prg.ram.banks_8k_plus = 1;

	info.mapper.extend_rd = TRUE;
	info.mapper.extend_wr = TRUE;
	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
}
void extcl_after_mapper_init_LH10(void) {
	// posso farlo solo dopo il map_prg_ram_init();
	lh10_update();
}
void extcl_cpu_wr_mem_LH10(WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x6000:
		case 0x6001:
		case 0x7000:
		case 0x7001:
			return;
		case 0xC000:
		case 0xC001:
		case 0xD000:
		case 0xD001:
			lh10_prg_C000[address & 0x1FFF] = value;
			return;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xE000:
		case 0xF000:
			lh10.ind = value & 0x07;
			return;
		case 0x8001:
		case 0x9001:
		case 0xA001:
		case 0xB001:
		case 0xE001:
		case 0xF001:
			lh10.reg[lh10.ind] = value;
			lh10_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_LH10(WORD address, BYTE openbus, BYTE before) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			return (lh10_prg_6000[address & 0x1FFF]);
		case 0xC000:
		case 0xD000:
			return (lh10_prg_C000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_LH10(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, lh10.ind);
	save_slot_ele(mode, slot, lh10.reg);

	if (mode == SAVE_SLOT_READ) {
		lh10_update();
	}

	return (EXIT_OK);
}

static void INLINE lh10_update(void) {
	WORD value;

	// 0x6000 - 0x7000
	value = 0xFE;
	control_bank(info.prg.rom[0].max.banks_8k)
	lh10_prg_6000 = prg_chip_byte_pnt(0, value << 13);

	// 0x8000 - 0x9000
	value = lh10.reg[6];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);
	prg.rom_8k[0] = prg_chip_byte_pnt(prg.rom_chip[0], mapper.rom_map_to[0] << 13);

	// 0xA000 - 0xB000
	value = lh10.reg[7];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);
	prg.rom_8k[1] = prg_chip_byte_pnt(prg.rom_chip[0], mapper.rom_map_to[1] << 13);

	// 0xC000 - 0xD000
	lh10_prg_C000 = &prg.ram_plus_8k[0];
}
