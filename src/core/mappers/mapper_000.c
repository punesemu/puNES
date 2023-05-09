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

INLINE static void prg_fix_000(void);
INLINE static void wram_fix_000(void);

struct _m000tmp {
	BYTE nrom368;
} m000tmp;

void map_init_000(void) {
	EXTCL_AFTER_MAPPER_INIT(000);
	EXTCL_CPU_WR_MEM(000);
	EXTCL_SAVE_MAPPER(000);

	// Alter Ego (World) (Aftermarket) (Homebrew) (Alt).nes
	m000tmp.nrom368 = (info.prg.rom.banks_16k == 3);
}
void extcl_after_mapper_init_000(void) {
	prg_fix_000();
	wram_fix_000();
}
void extcl_cpu_wr_mem_000(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_save_mapper_000(UNUSED(BYTE mode), UNUSED(BYTE slot), UNUSED(FILE *fp)) {
	if (mode == SAVE_SLOT_READ) {
		wram_fix_000();
	}
	return (EXIT_OK);
}

INLINE static void prg_fix_000(void) {
	WORD bank = 0;

	if (m000tmp.nrom368) {
		bank = 1;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = 2;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	} else {
		bank = 0;
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void wram_fix_000(void) {
	if (m000tmp.nrom368) {
		wram_map_prg_rom_16k(0x4000, 0);
	} else {
		wram_map_disable_8k(0x4000);
	}
}
