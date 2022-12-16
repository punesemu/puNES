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

INLINE static void prg_fix_538(void);

struct _m538 {
	BYTE reg;
} m538;
struct _m538tmp {
	BYTE *prg_6000;
} m538tmp;

void map_init_538(void) {
	EXTCL_AFTER_MAPPER_INIT(538);
	EXTCL_CPU_WR_MEM(538);
	EXTCL_CPU_RD_MEM(538);
	EXTCL_SAVE_MAPPER(538);
	mapper.internal_struct[0] = (BYTE *)&m538;
	mapper.internal_struct_size[0] = sizeof(m538);

	memset(&m538, 0x00, sizeof(m538));
}
void extcl_after_mapper_init_538(void) {
	prg_fix_538();
}
void extcl_cpu_wr_mem_538(WORD address, BYTE value) {
	if ((address >= 0xC000) && (address <= 0xDFFF)) {
		m538.reg = value;
		prg_fix_538();
	}
}
BYTE extcl_cpu_rd_mem_538(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (m538tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_538(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m538.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_538();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_538(void) {
	WORD bank;

	bank = m538.reg | 0x01;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m538tmp.prg_6000 = prg_pnt(bank << 13);

	bank = (m538.reg & 0x01) && (~m538.reg & 0x08) ? 0x0A : m538.reg & 0xFE;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = 0x0D;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = 0x0E;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0x0F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
