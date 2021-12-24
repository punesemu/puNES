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

void map_init_CC_21(void) {
	EXTCL_CPU_WR_MEM(CC_21);

	map_prg_rom_8k(4, 0, 0);
	extcl_cpu_wr_mem_CC_21(0x0000, 0x00);
}
void extcl_cpu_wr_mem_CC_21(WORD address, BYTE value) {
	DBWORD bank;
	WORD vl = address;

	if (address == 0x8000) {
		vl = value;
	}

	if (chr.chip[0].size == 8192) {
		vl = vl & 0x01;
		_control_bank(vl, info.chr.rom[0].max.banks_4k)
		bank = vl << 12;
		chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
		chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
		chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
		chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);

		chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
	} else {
		vl = vl & 0x01;
		_control_bank(vl, info.chr.rom[0].max.banks_8k)
		bank = vl << 13;
		chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
		chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
		chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
		chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
		chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
	}

	if (vl & 0x01) {
		mirroring_SCR1();
	} else {
		mirroring_SCR0();
	}
}
