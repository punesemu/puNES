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

BYTE dipswitch[2][14] = {
	{ 0, 8, 2, 1, 3, 5, 4, 6, 9, 10, 11, 13, 14, 7 },
	{ 8, 3, 5, 4, 6, 9, 10, 11, 13, 14, 7, 1, 2, 0 }
};
struct _m288 {
	WORD reg;
} m288;
struct _m288tmp {
	BYTE index;
	BYTE dipswitch;
} m288tmp;

void map_init_288(void) {
	EXTCL_AFTER_MAPPER_INIT(288);
	EXTCL_CPU_WR_MEM(288);
	EXTCL_CPU_RD_MEM(288);
	EXTCL_SAVE_MAPPER(288);
	mapper.internal_struct[0] = (BYTE *)&m288;
	mapper.internal_struct_size[0] = sizeof(m288);

	memset(&m288, 0x00, sizeof(m288));

	if (info.crc32.prg == 0xEE24B155) {
		// 64-in-1 (CF-015).nes
		m288tmp.index = 1;
	} else {
		// 21-in-1 (GA-003).nes
		m288tmp.index = 0;
	}

	if (info.reset == RESET) {
		if (++m288tmp.dipswitch >= 14) {
			m288tmp.dipswitch = 0;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m288tmp.dipswitch = 0;
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_288(void) {
	extcl_cpu_wr_mem_288(0x0000, 0x00);
}
void extcl_cpu_wr_mem_288(WORD address, BYTE value) {
	DBWORD bank;

	m288.reg = address;

	value = (address >> 3) & 0x03;
	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	bank = address & 0x07;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank | 0x0000);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
BYTE extcl_cpu_rd_mem_288(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (m288.reg & 0x0020)) {
		return (prg_rom_rd((address | dipswitch[m288tmp.index][m288tmp.dipswitch])));
	}
	return (openbus);
}
BYTE extcl_save_mapper_288(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m288.reg);

	return (EXIT_OK);
}
