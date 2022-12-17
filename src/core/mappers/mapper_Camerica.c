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

void map_init_Camerica(void) {
	switch (info.mapper.submapper) {
		default:
			EXTCL_CPU_WR_MEM(Camerica_BF9093);
			break;
		case 1:
			EXTCL_CPU_WR_MEM(Camerica_BF9097);
			break;
	}
}
void extcl_cpu_wr_mem_Camerica_BF9093(UNUSED(WORD address), BYTE value) {
	control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
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
			control_bank_with_AND(0x07, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}

void map_init_Camerica_BF9096(void) {
	EXTCL_CPU_WR_MEM(Camerica_BF9096);

	if ((info.crc32.prg == 0x4B40CBD9) || // Pegasus 4-in-1 (Unl) [!].nes
		(info.crc32.prg == 0x6C040686) || // Quattro Adventure (Camerica) [!p].nes
		(info.crc32.prg == 0x60DAAB04) || // Quattro Adventure (Camerica) [T+Pol(T.Island Only)].nes
		(info.crc32.prg == 0x62EF6C79) || // Quattro Sports (Camerica) [!p].nes
		(info.crc32.prg == 0xA045FE1D)) { // Super Sports Challenge (CodeMasters) (V2 Plug-Thru Cart).nes
		info.mapper.submapper = 1;
	}
	if ((info.crc32.prg == 0xB89888C9) || // Quattro Adventure (USA) (Unl).nes
		(info.crc32.prg == 0x792070A9) || // Quattro Arcade (USA) (Unl).nes
		(info.crc32.prg == 0xCCCAF368)) { // Quattro Sports (USA) (Unl).nes
		info.mapper.submapper = 0;
	}
}
void extcl_cpu_wr_mem_Camerica_BF9096(WORD address, BYTE value) {
	BYTE base;

	switch ((address >> 12) & 0x0C) {
		case 0x08: {
			BYTE low = (mapper.rom_map_to[0] >> 1);

			if (info.mapper.submapper == 1) {
				base = ((value & 0x10) >> 2) | (value & 0x08);
			} else {
				base = (value & 0x18) >> 1;
			}

			value = base | ((low & 0x07) >> 1);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);

			value = base | 0x03;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		}
		default:
			value = ((mapper.rom_map_to[0] & 0x18) >> 1) | (value & 0x03);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			break;
	}
	map_prg_rom_8k_update();
}

void map_init_Camerica_GoldenFive(void) {
	EXTCL_CPU_WR_MEM(Camerica_GoldenFive);
	if (info.reset >= HARD) {
		map_prg_rom_8k(2, 2, 0x0F);
	}
}
void extcl_cpu_wr_mem_Camerica_GoldenFive(WORD address, BYTE value) {
	BYTE base;

	switch ((address >> 12) & 0x0C) {
		case 0x08:
			if (value & 0x08) {
				base = (value << 4) & 0x70;

				value = base | (mapper.rom_map_to[0] & 0x1E) >> 1;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, value);

				value = base | 0x0F;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 2, value);
			}
			break;
		default:
			value = ((mapper.rom_map_to[0] & 0xE0) >> 1) | (value & 0x0F);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			break;
	}
	map_prg_rom_8k_update();
}
