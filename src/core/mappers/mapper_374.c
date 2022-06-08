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

INLINE static void prg_fix_374(void);
INLINE static void chr_fix_374(void);

struct _m374 {
	BYTE index;
} m374;

void map_init_374(void) {
	info.mapper.submapper = MAP374;
	map_init_MMC1();

	EXTCL_AFTER_MAPPER_INIT(374);
	EXTCL_SAVE_MAPPER(374);
	mapper.internal_struct[0] = (BYTE *)&m374;
	mapper.internal_struct_size[0] = sizeof(m374);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	if (info.reset == RESET) {
		m374.index++;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m374.index = 0;
	}

	mmc1.prg_mask = 0x07;
	mmc1.prg_upper = m374.index << 3;
	mmc1.chr_upper = m374.index << 5;
}
void extcl_after_mapper_init_374(void) {
	prg_fix_374();
	chr_fix_374();
}
BYTE extcl_save_mapper_374(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m374.index);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_374(void) {
	WORD bank;

	bank = mmc1.prg_upper | (mmc1.prg0 & mmc1.prg_mask);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = mmc1.prg_upper | mmc1.prg_mask;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_374(void) {
	DBWORD bank;

	bank = mmc1.chr_upper & 0x1F;
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
