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

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

void map_init_Hes(void) {
	EXTCL_CPU_WR_MEM(Hes);

	info.mapper.extend_wr = TRUE;

	if (info.reset >= HARD) {
		if (info.prg.rom.max.banks_32k != 0xFFFF) {
			map_prg_rom_8k(4, 0, 0);
		}
	}
}
void extcl_cpu_wr_mem_Hes(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if ((address & 0x0100) == 0x0100) {
		const BYTE save = value;
		DBWORD bank;

		if (value & 0x80) {
			mirroring_V();
		} else {
			mirroring_H();
		}

		if (info.prg.rom.max.banks_32k != 0xFFFF) {
			value = (value >> 3) & 0x07;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
		}

		value = ((save >> 3) & 0x08) | (save & 0x07);
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
