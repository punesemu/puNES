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

INLINE static void prg_fix_389(void);
INLINE static void chr_fix_389(void);
INLINE static void mirroring_fix_389(void);

struct _m389 {
	WORD reg[3];
} m389;

void map_init_389(void) {
	EXTCL_AFTER_MAPPER_INIT(389);
	EXTCL_CPU_WR_MEM(389);
	EXTCL_SAVE_MAPPER(389);
	mapper.internal_struct[0] = (BYTE *)&m389;
	mapper.internal_struct_size[0] = sizeof(m389);

	memset(&m389, 0x00, sizeof(m389));
}
void extcl_after_mapper_init_389(void) {
	prg_fix_389();
	chr_fix_389();
	mirroring_fix_389();
}
void extcl_cpu_wr_mem_389(WORD address, UNUSED(BYTE value)) {
	switch (address & 0xF000) {
		case 0x8000:
			m389.reg[0] = address & 0xFF;
			prg_fix_389();
			mirroring_fix_389();
			break;
		case 0x9000:
			m389.reg[1] = address & 0xFF;
			prg_fix_389();
			chr_fix_389();
			break;
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m389.reg[2] = address & 0x0F;
			prg_fix_389();
			chr_fix_389();
			break;
	}
}
BYTE extcl_save_mapper_389(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m389.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_389(void) {
	WORD bank;

	if (m389.reg[1] & 0x02) {
		bank = (m389.reg[0] >> 2) | ((m389.reg[2] & 0x0C) >> 2);
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = (m389.reg[0] >> 2) | 0x03;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	} else {
		bank = m389.reg[0] >> 3;
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_389(void) {
	DBWORD bank;

	bank = ((m389.reg[1] & 0x38) >> 1) | (m389.reg[2] & 0x03);
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
INLINE static void mirroring_fix_389(void) {
	if (m389.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
