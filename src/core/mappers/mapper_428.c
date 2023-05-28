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
#include "mem_map.h"
#include "info.h"
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_428(void);
INLINE static void chr_fix_428(void);
INLINE static void mirroring_fix_428(void);

struct _m428 {
	BYTE reg[5];
	BYTE data;
} m428;
struct _m428tmp {
	BYTE dipswitch;
} m428tmp;

void map_init_428(void) {
	EXTCL_AFTER_MAPPER_INIT(428);
	EXTCL_CPU_WR_MEM(428);
	EXTCL_CPU_RD_MEM(428);
	EXTCL_SAVE_MAPPER(428);
	mapper.internal_struct[0] = (BYTE *)&m428;
	mapper.internal_struct_size[0] = sizeof(m428);

	memset(&m428, 0x00, sizeof(m428));

	if (info.reset == RESET) {
		m428tmp.dipswitch = (m428tmp.dipswitch + 1) & 0x03;
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m428tmp.dipswitch = 0;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_428(void) {
	prg_fix_428();
	chr_fix_428();
	mirroring_fix_428();
}
void extcl_cpu_wr_mem_428(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			m428.reg[address & 0x03] = value;
			prg_fix_428();
			chr_fix_428();
			mirroring_fix_428();
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m428.reg[4] = value;
			chr_fix_428();
			break;
	}
}
BYTE extcl_cpu_rd_mem_428(WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return ((openbus & 0xFC) | (m428tmp.dipswitch & 0x03));
	}
	return (openbus);
}
BYTE extcl_save_mapper_428(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m428.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_428(void) {
	WORD bank;

	if (m428.reg[1] & 0x10) {
		bank = m428.reg[1] >> 6;
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		bank = m428.reg[1] >> 5;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_428(void) {
	DBWORD bank = ((m428.reg[1] & 0x07) & ~(m428.reg[2] >> 6)) | (m428.reg[4] & (m428.reg[2] >> 6));

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
INLINE static void mirroring_fix_428(void) {
	if (m428.reg[1] & 0x08) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
