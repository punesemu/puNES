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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"
#include "cpu.h"

struct _m60 {
	BYTE index;
} m60;

void map_init_60(void) {
	EXTCL_CPU_WR_MEM(60);
	EXTCL_SAVE_MAPPER(60);
	mapper.internal_struct[0] = (BYTE *)&m60;
	mapper.internal_struct_size[0] = sizeof(m60);

	if (info.reset >= HARD) {
		m60.index = 0;
	} else {
		m60.index++;
		m60.index = m60.index & 0x03;
	}

	{
		BYTE value;
		DBWORD bank;

		value = m60.index;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);

		value = m60.index;
		control_bank(info.chr.rom.max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}
void extcl_cpu_wr_mem_60(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_save_mapper_60(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m60.index);

	return (EXIT_OK);
}
