/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

void map_init_Camerica(void) {
	switch (info.mapper.submapper) {
		case BF9096:
			EXTCL_CPU_WR_MEM(Camerica_BF9096);
			break;
		case BF9097:
			EXTCL_CPU_WR_MEM(Camerica_BF9097);
			break;
		case GOLDENFIVE:
			EXTCL_CPU_WR_MEM(Camerica_GoldenFive);
			if (info.reset >= HARD) {
				map_prg_rom_8k(2, 2, 0x0F);
			}
			break;
		default:
			EXTCL_CPU_WR_MEM(Camerica_BF9093);
			break;
	}
}
void extcl_cpu_wr_mem_Camerica_BF9093(WORD address, BYTE value) {
	control_bank_with_AND(0x0F, info.prg.rom[0].max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}
void extcl_cpu_wr_mem_Camerica_BF9096(WORD address, BYTE value) {
	BYTE base;

	switch ((address >> 12) & 0x0C) {
		case 0x08: {
			BYTE low = (mapper.rom_map_to[0] >> 1);

			if (info.id == PEGASUS4IN1) {
				base = ((value & 0x10) >> 2) | (value & 0x08);
				map_prg_rom_8k(2, 0, base | ((low & 0x07) >> 1));
			} else {
				base = (value & 0x18) >> 1;
				map_prg_rom_8k(2, 0, base | ((low & 0x07) >> 1));
			}
			map_prg_rom_8k(2, 2, base | 0x03);
			break;
		}
		default:
			map_prg_rom_8k(2, 0, ((mapper.rom_map_to[0] & 0x18) >> 1) | (value & 0x03));
			break;
	}
	map_prg_rom_8k_update();
}
void extcl_cpu_wr_mem_Camerica_BF9097(WORD address, BYTE value) {
	switch ((address >> 12) & 0x0C) {
		case 0x08:
			if (value & 0x10) {
				mirroring_SCR0();
			} else {
				mirroring_SCR1();
			}
			return;
		default:
			control_bank_with_AND(0x07, info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
void extcl_cpu_wr_mem_Camerica_GoldenFive(WORD address, BYTE value) {
	BYTE base;

	switch ((address >> 12) & 0x0C) {
		case 0x08:
			if (value & 0x08) {
				base = (value << 4) & 0x70;
				map_prg_rom_8k(2, 0, base | (mapper.rom_map_to[0] & 0x1E) >> 1);
				map_prg_rom_8k(2, 2, base | 0x0F);
			}
			break;
		default:
			map_prg_rom_8k(2, 0, ((mapper.rom_map_to[0] & 0xE0) >> 1) | (value & 0x0F));
			break;
	}
	map_prg_rom_8k_update();
}
