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

void map_init_LH51(void) {
	EXTCL_AFTER_MAPPER_INIT(LH51);
	EXTCL_CPU_WR_MEM(LH51);

	info.prg.ram.banks_8k_plus = 1;
}
void extcl_after_mapper_init_LH51(void) {
	extcl_cpu_wr_mem_LH51(0x8000, 0);
}
void extcl_cpu_wr_mem_LH51(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			value = value & 0x0F;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			value = 0x0D;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			value = 0x0E;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			value = 0x0F;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
			map_prg_rom_8k_update();
			return;
		case 0xE000:
			if (value & 0x08) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
}
