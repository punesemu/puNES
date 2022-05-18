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

INLINE static void prg_fix_CTC09(void);
INLINE static void chr_fix_CTC09(void);
INLINE static void mirroring_fix_CTC09(void);

struct _ctc09 {
	BYTE reg[2];
} ctc09;

void map_init_CTC09(void) {
	EXTCL_AFTER_MAPPER_INIT(CTC09);
	EXTCL_CPU_WR_MEM(CTC09);
	EXTCL_SAVE_MAPPER(CTC09);
	mapper.internal_struct[0] = (BYTE *)&ctc09;
	mapper.internal_struct_size[0] = sizeof(ctc09);

	if (info.reset >= HARD) {
		memset(&ctc09, 0x00, sizeof(ctc09));
	}
}
void extcl_after_mapper_init_CTC09(void) {
	prg_fix_CTC09();
	chr_fix_CTC09();
	mirroring_fix_CTC09();
}
void extcl_cpu_wr_mem_CTC09(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			ctc09.reg[0] = value;
			chr_fix_CTC09();
			break;
		case 0xC000:
		case 0xE000:
			ctc09.reg[1] = value;
			prg_fix_CTC09();
			mirroring_fix_CTC09();
			break;
	}
}
BYTE extcl_save_mapper_CTC09(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ctc09.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_CTC09(void) {
	BYTE value;

	if (ctc09.reg[1] & 0x10) {
		value = ((ctc09.reg[1] & 0x07) << 1) | ((ctc09.reg[1] & 0x08) >> 3);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = (ctc09.reg[1] & 0x07);
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_CTC09(void) {
	DBWORD bank = ctc09.reg[0] & 0x0F;

	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank | 0x0000);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_CTC09(void) {
	if (ctc09.reg[1] & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
