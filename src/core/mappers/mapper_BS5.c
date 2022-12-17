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

struct _bs5mp {
	BYTE reset;
} bs5tmp;

void map_init_BS5(void) {
	EXTCL_CPU_WR_MEM(BS5);

	if (info.reset >= HARD) {
		bs5tmp.reset = 0;
	} else if (info.reset == RESET) {
		bs5tmp.reset++;
		bs5tmp.reset = bs5tmp.reset & 0x03;
	}

	{
		BYTE i;

		for (i = 0; i < 4; i++) {
			BYTE value = 0x0F;

			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, i, value);

			chr.bank_1k[(i << 1)] = chr_pnt(0);
			chr.bank_1k[(i << 1) | 0x01] = chr_pnt(0x0400);
		}
	}

	mirroring_V();
}
void extcl_cpu_wr_mem_BS5(WORD address, BYTE value) {
	BYTE base = (address & 0x0C00) >> 10;

	switch (address & 0x7000) {
		case 0x0000:
		case 0x1000: {
			DBWORD bank;

			base <<= 1;
			value = address & 0x1F;
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[base] = chr_pnt(bank);
			chr.bank_1k[base | 0x01] = chr_pnt(bank | 0x0400);
			return;
		}
		case 0x2000:
		case 0x3000:
			if (address & (1 << (bs5tmp.reset + 4))) {
				value = address & 0x0F;
				control_bank(info.prg.rom.max.banks_8k)
				map_prg_rom_8k(1, base, value);
				map_prg_rom_8k_update();
			}
			return;
	}
}
