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

INLINE static void prg_fix_299(void);
INLINE static void chr_fix_299(void);
INLINE static void mirroring_fix_299(void);

struct _m299 {
	BYTE reg;
} m299;

void map_init_299(void) {
	EXTCL_AFTER_MAPPER_INIT(299);
	EXTCL_CPU_WR_MEM(299);
	EXTCL_SAVE_MAPPER(299);
	mapper.internal_struct[0] = (BYTE *)&m299;
	mapper.internal_struct_size[0] = sizeof(m299);

	memset(&m299, 0x00, sizeof(m299));
}
void extcl_after_mapper_init_299(void) {
	prg_fix_299();
	chr_fix_299();
	mirroring_fix_299();
}
void extcl_cpu_wr_mem_299(UNUSED(WORD address), BYTE value) {
	m299.reg = value;
	prg_fix_299();
	chr_fix_299();
	mirroring_fix_299();
}
BYTE extcl_save_mapper_299(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m299.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_299(void) {
	WORD bank = (m299.reg & 0x70) >> 4;

	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_299(void) {
	DBWORD bank = ((m299.reg & 0x70) >> 2) | (m299.reg & 0x03);

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
INLINE static void mirroring_fix_299(void) {
	if (m299.reg & 0x80) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
