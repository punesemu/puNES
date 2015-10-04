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

void map_init_156(void) {
	EXTCL_CPU_WR_MEM(156);

	mirroring_SCR0();
}
void extcl_cpu_wr_mem_156(WORD address, BYTE value) {
	switch (address) {
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[address & 0x0007] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xC008:
		case 0xC009:
		case 0xC00A:
		case 0xC00B:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[(address & 0x000F) - 4] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xC010:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
