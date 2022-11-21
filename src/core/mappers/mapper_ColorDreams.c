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

void map_init_ColorDreams(void) {
	EXTCL_CPU_WR_MEM(ColorDreams);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_ColorDreams(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD chr_bank;

	/* bus conflict */
	if (info.mapper.submapper != CD_NO_CONFLCT) {
		save = value &= prg_rom_rd(address);
	}

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = (save & 0xF0) >> 4;
	control_bank(info.chr.rom.max.banks_8k)
	chr_bank = value << 13;
	chr.bank_1k[0] = chr_pnt(chr_bank);
	chr.bank_1k[1] = chr_pnt(chr_bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(chr_bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(chr_bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(chr_bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(chr_bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(chr_bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(chr_bank | 0x1C00);
}
