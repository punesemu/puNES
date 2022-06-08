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

INLINE static void prg_fix_381(void);

struct _m381 {
	BYTE index;
	BYTE reg;
} m381;

void map_init_381(void) {
	EXTCL_AFTER_MAPPER_INIT(381);
	EXTCL_CPU_WR_MEM(381);
	EXTCL_SAVE_MAPPER(381);
	mapper.internal_struct[0] = (BYTE *)&m381;
	mapper.internal_struct_size[0] = sizeof(m381);

	if (info.reset == RESET) {
		m381.index++;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m381.index = 0;
	}
}
void extcl_after_mapper_init_381(void) {
	prg_fix_381();
}
void extcl_cpu_wr_mem_381(UNUSED(WORD address), BYTE value) {
	m381.reg = value;
	prg_fix_381();
}
BYTE extcl_save_mapper_381(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m381.index);
	save_slot_ele(mode, slot, m381.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_381(void) {
	WORD bank;

	bank = (((m381.reg << 1) | (m381.reg >> 4)) & 0x0F) | (m381.index << 4);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = (m381.index << 4) | 0x0F;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
