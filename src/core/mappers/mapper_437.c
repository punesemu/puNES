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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_437(void);
INLINE static void mirroring_fix_437(void);

struct _m437 {
	WORD reg[2];
} m437;

void map_init_437(void) {
	EXTCL_AFTER_MAPPER_INIT(437);
	EXTCL_CPU_WR_MEM(437);
	EXTCL_SAVE_MAPPER(437);
	mapper.internal_struct[0] = (BYTE *)&m437;
	mapper.internal_struct_size[0] = sizeof(m437);

	if (info.reset >= HARD) {
		memset(&m437, 0x00, sizeof(m437));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_437(void) {
	prg_fix_437();
	mirroring_fix_437();
}
void extcl_cpu_wr_mem_437(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m437.reg[0] = address;
		prg_fix_437();
		mirroring_fix_437();
	} else if (address >= 0x8000) {
		m437.reg[1] = value;
		prg_fix_437();
	}
}
BYTE extcl_save_mapper_437(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m437.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_437(void) {
	WORD base = (m437.reg[0] & 0x07) << 3;
	WORD bank;

	bank = base | (m437.reg[1] & 0x07);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = base | 0x07;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_437(void) {
	if (m437.reg[0] & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
