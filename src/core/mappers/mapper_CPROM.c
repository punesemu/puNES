/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

void map_init_CPROM(void) {
	/* forzo i numeri di banchi della chr rom */
	info.chr.rom[0].banks_8k = 2;
	info.chr.rom[0].banks_4k = 4;
	info.chr.rom[0].banks_1k = 16;
	/* quindi setto nuovamente i valori massimi dei banchi */
	map_set_banks_max_prg(0);
	map_set_banks_max_chr(0);

	if (info.reset >= HARD) {
		chr.bank_1k[4] = chr_chip_byte_pnt(0, 0x0000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, 0x0400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, 0x0800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, 0x0C00);
	}

	EXTCL_CPU_WR_MEM(CPROM);
}
void extcl_cpu_wr_mem_CPROM(WORD address, BYTE value) {
	DBWORD bank;

	/* bus conflict */
	value &= prg_rom_rd(address);

	control_bank_with_AND(0x03, info.chr.rom[0].max.banks_4k)
	bank = value << 12;
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
}
