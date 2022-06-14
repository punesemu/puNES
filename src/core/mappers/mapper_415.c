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

INLINE static void prg_fix_415(void);
INLINE static void mirroring_fix_415(void);

struct _m415 {
	BYTE reg;
} m415;
struct _m415tmp {
	BYTE *prg_6000;
} m415tmp;

void map_init_415(void) {
	EXTCL_AFTER_MAPPER_INIT(415);
	EXTCL_CPU_WR_MEM(415);
	EXTCL_CPU_RD_MEM(415);
	EXTCL_SAVE_MAPPER(415);
	mapper.internal_struct[0] = (BYTE *)&m415;
	mapper.internal_struct_size[0] = sizeof(m415);

	memset(&m415, 0x00, sizeof(m415));
}
void extcl_after_mapper_init_415(void) {
	prg_fix_415();
	mirroring_fix_415();
}
void extcl_cpu_wr_mem_415(UNUSED(WORD address), BYTE value) {
	m415.reg = value;
	prg_fix_415();
	mirroring_fix_415();
}
BYTE extcl_cpu_rd_mem_415(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0x6000) {
		return (m415tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}

BYTE extcl_save_mapper_415(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m415.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_415();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_415(void) {
	WORD bank;

	bank = m415.reg & 0x0F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m415tmp.prg_6000 = prg_pnt(bank << 13);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_415(void) {
	if (m415.reg & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
