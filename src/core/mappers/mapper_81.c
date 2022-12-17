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

INLINE static void prg_fix_81(void);
INLINE static void chr_fix_81(void);

struct _m81 {
	WORD reg;
} m81;

void map_init_81(void) {
	EXTCL_AFTER_MAPPER_INIT(81);
	EXTCL_CPU_WR_MEM(81);
	EXTCL_SAVE_MAPPER(81);
	mapper.internal_struct[0] = (BYTE *)&m81;
	mapper.internal_struct_size[0] = sizeof(m81);

	memset(&m81, 0x00, sizeof(m81));
}
void extcl_after_mapper_init_81(void) {
	prg_fix_81();
	chr_fix_81();
}
void extcl_cpu_wr_mem_81(WORD address, UNUSED(BYTE value)) {
	m81.reg = address;
	prg_fix_81();
	chr_fix_81();
}

BYTE extcl_save_mapper_81(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m81.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_81(void) {
	WORD bank;

	bank = (m81.reg & 0x000C) >> 2;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_81(void) {
	DBWORD bank;

	bank = m81.reg & 0x0003;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank = bank << 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
