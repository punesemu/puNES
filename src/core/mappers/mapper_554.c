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

INLINE static void prg_fix_554(void);
INLINE static void chr_fix_554(void);

struct _m554 {
	WORD reg;
} m554;
struct _m554tmp {
	BYTE *prg_6000;
} m554tmp;

void map_init_554(void) {
	EXTCL_AFTER_MAPPER_INIT(554);
	EXTCL_CPU_WR_MEM(554);
	EXTCL_CPU_RD_MEM(554);
	EXTCL_SAVE_MAPPER(554);
	mapper.internal_struct[0] = (BYTE *)&m554;
	mapper.internal_struct_size[0] = sizeof(m554);

	memset(&m554, 0x00, sizeof(m554));

	info.mapper.extend_rd = TRUE;
	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
}
void extcl_after_mapper_init_554(void) {
	prg_fix_554();
	chr_fix_554();
}
void extcl_cpu_wr_mem_554(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_554(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			return (m554tmp.prg_6000[address & 0x1FFF]);
		case 0xC000:
			if ((address >= 0xCAB6) && (address <= 0xCAD7)) {
				m554.reg = (address & 0x3C) >> 2;
				extcl_after_mapper_init_554();
			}
			break;
		case 0xE000:
			address &= 0xFFFE;
			if ((address == 0xEBE2) || (address == 0xEE32)) {
				m554.reg = (address & 0x3C) >> 2;
				extcl_after_mapper_init_554();
			}
			break;
		case 0xF000:
			address &= 0xFFFE;
			if (address == 0xFFFC) {
				m554.reg = (address & 0x3C) >> 2;
				extcl_after_mapper_init_554();
			}
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_554(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m554.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_554();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_554(void) {
	WORD bank;

	bank = m554.reg;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m554tmp.prg_6000 = prg_pnt(bank << 13);

	bank = 0x0A;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = 0x0B;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = 0x06;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0x07;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_554(void) {
	DBWORD bank;

	bank = m554.reg;
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
