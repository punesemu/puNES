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
#include "mem_map.h"
#include "info.h"
#include "save_slot.h"

struct _bb {
	BYTE reg;
} bb;
struct _bbtmp {
	BYTE *prg_6000;
} bbtmp;

void map_init_BB(void) {
	EXTCL_CPU_WR_MEM(BB);
	EXTCL_CPU_RD_MEM(BB);
	EXTCL_SAVE_MAPPER(BB);
	mapper.internal_struct[0] = (BYTE *) &bb;
	mapper.internal_struct_size[0] = sizeof(bb);

	{
		BYTE value = 0xFF;

		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	bb.reg = 0xFF;
	_control_bank(bb.reg, info.prg.rom[0].max.banks_8k)
	bbtmp.prg_6000 = prg_chip_byte_pnt(0, bb.reg << 13);
}
void extcl_cpu_wr_mem_BB(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	if ((address & 0x9000) == 0x8000) {
		value = value & 0x03;
		control_bank(info.prg.rom[0].max.banks_8k)
		bbtmp.prg_6000 = prg_chip_byte_pnt(0, value << 13);
		bb.reg = value;
		value = save;
	} else {
		value = value & 0x01;
	}

	control_bank(info.chr.rom[0].max.banks_8k);
	bank = value << 13;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
}
BYTE extcl_cpu_rd_mem_BB(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (bbtmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_BB(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bb.reg);

	if (mode == SAVE_SLOT_READ) {
		bbtmp.prg_6000 = prg_chip_byte_pnt(0, bb.reg << 13);
	}

	return (EXIT_OK);
}
