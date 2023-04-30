/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "mem_map.h"
#include "save_slot.h"

static const SBYTE dipswitch_59[][4] = {
	{  0,  1,  2,  3 }, // 0
	{  0,  1,  2, -1 }, // 1
};

struct _m059 {
	WORD reg;
} m059;
struct _m059tmp {
	BYTE select;
	BYTE index;
	WORD dipswitch;
} m059tmp;

void map_init_059(void) {
	EXTCL_CPU_WR_MEM(059);
	EXTCL_CPU_RD_MEM(059);
	EXTCL_SAVE_MAPPER(059);
	mapper.internal_struct[0] = (BYTE *)&m059;
	mapper.internal_struct_size[0] = sizeof(m059);

	m059.reg = 0;

	if (info.reset == RESET) {
		do {
			m059tmp.index = (m059tmp.index + 1) & 0x03;
		} while (dipswitch_59[m059tmp.select][m059tmp.index] < 0);
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0xED831F98) { // (VT-104) 2000 Super Aladdin.nes
			m059tmp.select = 1;
			m059tmp.index = 0;
		} else {
			m059tmp.select = 0;
			m059tmp.index = 0;
		}
	}

	m059tmp.dipswitch = dipswitch_59[m059tmp.select][m059tmp.index];

	info.mapper.extend_rd = TRUE;

	extcl_cpu_wr_mem_059(0x0000, 0);
}
void extcl_cpu_wr_mem_059(WORD address, BYTE value) {
	DBWORD bank;

	if (m059.reg & 0x0200) {
		return;
	}

	m059.reg = address;

	if (address & 0x80) {
		value = (address & 0x70) >> 4;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = (address & 0x60) >> 5;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();

	value = address & 0x07;
	control_bank(info.chr.rom.max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);

	if (address & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
BYTE extcl_cpu_rd_mem_059(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (m059.reg & 0x0100)) {
		return ((openbus & 0xFC) | m059tmp.dipswitch);
	}
	return (openbus);
}
BYTE extcl_save_mapper_059(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m059.reg);
	save_slot_ele(mode, slot, m059tmp.index);
	save_slot_ele(mode, slot, m059tmp.dipswitch);

	return (EXIT_OK);
}
