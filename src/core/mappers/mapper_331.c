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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_331(void);
INLINE static void chr_fix_331(void);
INLINE static void mirroring_fix_331(void);

struct _m331 {
	BYTE reg[3];
} m331;

void map_init_331(void) {
	EXTCL_AFTER_MAPPER_INIT(331);
	EXTCL_CPU_WR_MEM(331);
	EXTCL_SAVE_MAPPER(331);
	mapper.internal_struct[0] = (BYTE *)&m331;
	mapper.internal_struct_size[0] = sizeof(m331);

	memset(&m331, 0x00, sizeof(m331));
}
void extcl_after_mapper_init_331(void) {
	prg_fix_331();
	chr_fix_331();
	mirroring_fix_331();
}
void extcl_cpu_wr_mem_331(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0xA000:
			m331.reg[0] = value;
			break;
		case 0xC000:
			m331.reg[1] = value;
			break;
		case 0xE000:
			m331.reg[2] = value & 0x0F;
			break;
		default:
			return;
	}
	prg_fix_331();
	chr_fix_331();
	mirroring_fix_331();
}
BYTE extcl_save_mapper_331(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m331.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_331(void) {
	WORD base = (m331.reg[2] & 0x03) << 3;
	WORD bank[2] = { 0, 0 };

	if (m331.reg[2] & 0x08) {
		bank[0] = base | (m331.reg[0] & 0x06);
		bank[1] = base | (m331.reg[0] & 0x06) | 0x01;
	} else {
		bank[0] = base | (m331.reg[0] & 0x07);
		bank[1] = base | 0x07;
	}

	_control_bank(bank[0], info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank[0]);

	_control_bank(bank[1], info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank[1]);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_331(void) {
	WORD base = (m331.reg[2] & 0x03) << 5;
	DBWORD bank = 0;

	bank = base | (m331.reg[0] >> 3);
	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);

	bank = base | (m331.reg[1] >> 3);
	_control_bank(bank, info.chr.rom.max.banks_4k)
	bank <<= 12;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);
	chr.bank_1k[6] = chr_pnt(bank | 0x0800);
	chr.bank_1k[7] = chr_pnt(bank | 0x0C00);
}
INLINE static void mirroring_fix_331(void) {
	if (m331.reg[2] & 0x04) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
