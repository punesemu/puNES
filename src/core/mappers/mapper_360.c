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

INLINE static void prg_fix_360(void);
INLINE static void chr_fix_360(void);
INLINE static void mirroring_fix_360(void);

static BYTE dipswitch_360[] = {
	 0,  2,  3,  4,
	 5,  6,  7,  8,
	 9, 10, 11, 12,
	13, 14, 15, 16,
	17, 18, 19, 20,
	21, 22, 23, 24,
	25, 26, 27, 28,
	29, 30, 31
};

struct _m360 {
	struct _m360_dipswitch {
		BYTE actual;
		BYTE index;
	} dipswitch;
} m360;

void map_init_360(void) {
	EXTCL_AFTER_MAPPER_INIT(360);
	EXTCL_SAVE_MAPPER(360);
	mapper.internal_struct[0] = (BYTE *)&m360;
	mapper.internal_struct_size[0] = sizeof(m360);

	if (info.reset == RESET) {
		if (++m360.dipswitch.index >= LENGTH(dipswitch_360)) {
			m360.dipswitch.index = 0;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m360.dipswitch.index = 0;
	}
	m360.dipswitch.actual = dipswitch_360[m360.dipswitch.index];
}
void extcl_after_mapper_init_360(void) {
	prg_fix_360();
	chr_fix_360();
	mirroring_fix_360();
}
BYTE extcl_save_mapper_360(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m360.dipswitch.actual);
	save_slot_ele(mode, slot, m360.dipswitch.index);

	return (EXIT_OK);
}

INLINE static void prg_fix_360(void) {
	WORD bank;

	if (m360.dipswitch.actual < 2) {
		bank = m360.dipswitch.actual >> 1;
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		bank = m360.dipswitch.actual;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_360(void) {
	DBWORD bank = m360.dipswitch.actual;

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
INLINE static void mirroring_fix_360(void) {
	if (m360.dipswitch.actual & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
