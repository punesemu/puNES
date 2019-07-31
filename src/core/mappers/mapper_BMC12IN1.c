/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

void map_init_BMC12IN1(void) {
	EXTCL_CPU_WR_MEM(BMC12IN1);
	EXTCL_SAVE_MAPPER(BMC12IN1);
	mapper.internal_struct[0] = (BYTE *) &bmc12in1;
	mapper.internal_struct_size[0] = sizeof(bmc12in1);

	memset(&bmc12in1, 0x00, sizeof(bmc12in1));

	extcl_cpu_wr_mem_BMC12IN1(0xA000, 0x00);
}
void extcl_cpu_wr_mem_BMC12IN1(WORD address, BYTE value) {
	DBWORD bank;
	BYTE base;

	switch (address & 0xE000) {
		case 0xA000:
			bmc12in1.reg[0] = value;
			break;
		case 0xC000:
			bmc12in1.reg[1] = value;
			break;
		case 0xE000:
			bmc12in1.reg[2] = value & 0x0F;
			break;
		default:
			return;
	}

	base = (bmc12in1.reg[2] & 0x03) << 3;

	if (bmc12in1.reg[2] & 0x08) {
		value = base | (bmc12in1.reg[0] & 0x06);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		value = base | (bmc12in1.reg[0] & 0x06) | 0x01;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	} else {
		value = base | (bmc12in1.reg[0] & 0x07);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		value = base | 0x07;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	value = (bmc12in1.reg[0] >> 3) | (base << 2);
	control_bank(info.chr.rom[0].max.banks_4k)
	bank = value << 12;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
	value = (bmc12in1.reg[1] >> 3) | (base << 2);
	control_bank(info.chr.rom[0].max.banks_4k)
	bank = value << 12;
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);

	if (bmc12in1.reg[2] & 0x04) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
BYTE extcl_save_mapper_BMC12IN1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc12in1.reg);

	return (EXIT_OK);
}
