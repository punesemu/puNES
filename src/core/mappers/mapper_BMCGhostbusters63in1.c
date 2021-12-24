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

INLINE static void bmcghostbusters63in1_update_chr(void);

struct _bmcghostbusters63in1 {
	BYTE reg[2];
	BYTE index;
} bmcghostbusters63in1;
static const BYTE bmcghostbusters63in1_chip[4] = { 0, 0, 1, 2 };

void map_init_BMCGHOSTBUSTERS63IN1(void) {
	EXTCL_CPU_WR_MEM(BMCGHOSTBUSTERS63IN1);
	//EXTCL_CPU_RD_MEM(BMCGHOSTBUSTERS63IN1);
	EXTCL_SAVE_MAPPER(BMCGHOSTBUSTERS63IN1);
	if (!mapper.write_vram) {
		EXTCL_WR_CHR(BMCGHOSTBUSTERS63IN1);
	}

	mapper.internal_struct[0] = (BYTE *)&bmcghostbusters63in1;
	mapper.internal_struct_size[0] = sizeof(bmcghostbusters63in1);

	memset(&bmcghostbusters63in1, 0x00, sizeof(bmcghostbusters63in1));

	map_chr_ram_extra_init(0x2000);
	map_chr_ram_extra_reset();

	extcl_cpu_wr_mem_BMCGHOSTBUSTERS63IN1(0x0000, 0);

	//info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_BMCGHOSTBUSTERS63IN1(WORD address, BYTE value) {
	BYTE chip;

	bmcghostbusters63in1.reg[address & 0x01] = value;
	bmcghostbusters63in1.index = ((bmcghostbusters63in1.reg[0] & 0x80) >> 7) |
			((bmcghostbusters63in1.reg[1] & 0x01) << 1);

	chip = bmcghostbusters63in1_chip[bmcghostbusters63in1.index];
	_control_bank(chip, info.prg.max_chips)

	if (bmcghostbusters63in1.reg[0] & 0x20) {
		value = bmcghostbusters63in1.reg[0] & 0x1F;
		control_bank(info.prg.rom[chip].max.banks_16k)
		map_prg_rom_8k_chip(2, 0, value, chip);
		map_prg_rom_8k_chip(2, 2, value, chip);
	} else {
		value = (bmcghostbusters63in1.reg[0] >> 1) & 0x0F;
		control_bank(info.prg.rom[chip].max.banks_32k)
		map_prg_rom_8k_chip(4, 0, value, chip);
	}
	map_prg_rom_8k_update();

	bmcghostbusters63in1_update_chr();

	if (bmcghostbusters63in1.reg[0] & 0x40) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
//BYTE extcl_cpu_rd_mem_BMCGHOSTBUSTERS63IN1(WORD address, BYTE openbus, BYTE before) {
//	if ((address >= 0x8000) && (bmcghostbusters63in1.index == 1)) {
//	}
//	return (openbus);
//}
BYTE extcl_save_mapper_BMCGHOSTBUSTERS63IN1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmcghostbusters63in1.reg);
	save_slot_ele(mode, slot, bmcghostbusters63in1.index);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);

	if (mode == SAVE_SLOT_READ) {
		bmcghostbusters63in1_update_chr();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_BMCGHOSTBUSTERS63IN1(WORD address, BYTE value) {
	chr.extra.data[address] = value;
}

INLINE static void bmcghostbusters63in1_update_chr(void) {
	if (!mapper.write_vram && (bmcghostbusters63in1.reg[1] & 0x02)) {
		chr.bank_1k[0] = &chr.extra.data[0x0000];
		chr.bank_1k[1] = &chr.extra.data[0x0400];
		chr.bank_1k[2] = &chr.extra.data[0x0800];
		chr.bank_1k[3] = &chr.extra.data[0x0C00];
		chr.bank_1k[4] = &chr.extra.data[0x1000];
		chr.bank_1k[5] = &chr.extra.data[0x1400];
		chr.bank_1k[6] = &chr.extra.data[0x1800];
		chr.bank_1k[7] = &chr.extra.data[0x1C00];
	} else {
		chr.bank_1k[0] = chr_chip_byte_pnt(0, 0x0000);
		chr.bank_1k[1] = chr_chip_byte_pnt(0, 0x0400);
		chr.bank_1k[2] = chr_chip_byte_pnt(0, 0x0800);
		chr.bank_1k[3] = chr_chip_byte_pnt(0, 0x0C00);
		chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x1000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x1400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x1800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x1C00);
	}
}
