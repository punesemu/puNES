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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_414(void);
INLINE static void chr_fix_414(void);
INLINE static void mirroring_fix_414(void);

static const WORD dipswitch_414[5] = { 0x010, 0x000, 0x030, 0x070, 0x0F0 };

struct _m414 {
	WORD reg[2];
} m414;
struct _m414tmp {
	BYTE index;
	WORD dipswitch;
} m414tmp;

void map_init_414(void) {
	EXTCL_AFTER_MAPPER_INIT(414);
	EXTCL_CPU_WR_MEM(414);
	EXTCL_CPU_RD_MEM(414);
	EXTCL_SAVE_MAPPER(414);
	mapper.internal_struct[0] = (BYTE *)&m414;
	mapper.internal_struct_size[0] = sizeof(m414);

	memset(&m414, 0x00, sizeof(m414));

	if (info.reset == RESET) {
		m414tmp.index = (m414tmp.index + 1) >= 5 ? 0 : m414tmp.index + 1;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m414tmp.index = 0;
	}

	m414tmp.dipswitch = dipswitch_414[m414tmp.index];

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_414(void) {
	prg_fix_414();
	chr_fix_414();
	mirroring_fix_414();
}
void extcl_cpu_wr_mem_414(WORD address, BYTE value) {
	m414.reg[0] = address;
	// bus conflict
	m414.reg[1] = value & prg_rom_rd(address);
	prg_fix_414();
	chr_fix_414();
	mirroring_fix_414();
}
BYTE extcl_cpu_rd_mem_414(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0xC000) {
		if (!(m414.reg[0] & 0x0100) && (m414.reg[0] & m414tmp.dipswitch)) {
			return (0xFF);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_414(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m414.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_414(void) {
	WORD bank;

	if (m414.reg[0] & 0x2000) {
		bank = (m414.reg[0] & 0x0004) >> 2;
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		bank = (m414.reg[0] & 0x0006) >> 1;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_414(void) {
	DBWORD bank;

	bank = m414.reg[1] & 0x03;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_414(void) {
	if (m414.reg[0] & 0x0001) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
