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

void map_init_AxROM(void) {
	EXTCL_CPU_WR_MEM(AxROM);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	if (info.id == BBCARUNL) {
		mirroring_SCR0();
	} else {
		mirroring_SCR1();
	}
}
void extcl_cpu_wr_mem_AxROM(WORD address, BYTE value) {
	/* bus conflict */
	if (info.mapper.submapper == AMROM) {
		value &= prg_rom_rd(address);
	}

	if (value & 0x10) {
		mirroring_SCR0();
	} else {
		mirroring_SCR1();
	}

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
