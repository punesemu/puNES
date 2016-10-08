/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

void map_init_246(void) {
	EXTCL_CPU_WR_MEM(246);
	EXTCL_CPU_RD_MEM(246);

	if (info.reset >= HARD) {
		map_prg_rom_8k_reset();
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_246(WORD address, BYTE value) {
	BYTE reg, slot;
	DBWORD bank;

	if ((address < 0x6000) || (address > 0x67FF)) {
		return;
	}

	reg = address & 0x07;

	if (reg < 4) {
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, reg, value);
		map_prg_rom_8k_update();
		return;
	}

	slot = (reg - 4) << 1;
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[slot + 1] = chr_chip_byte_pnt(0, bank | 0x0400);
}
BYTE extcl_cpu_rd_mem_246(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x67FF)) {
		return (openbus);
	}

	return(before);
}
