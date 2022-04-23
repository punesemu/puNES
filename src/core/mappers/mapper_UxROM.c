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
#include "ines.h"

void map_init_UxROM(BYTE model) {
	switch (model) {
		case UNLROM:
			EXTCL_CPU_WR_MEM(UnlROM);
			break;
		case UXROM:
			EXTCL_CPU_WR_MEM(UxROM);
			break;
		case UXROMNBC:
			EXTCL_CPU_WR_MEM(UxROM);
			break;
		case UNL1XROM:
			EXTCL_CPU_WR_MEM(Unl1xROM);
			break;
		case UNROM180:
			EXTCL_CPU_WR_MEM(UNROM_180);
			break;
		case UNROM_BK2:
			EXTCL_CPU_WR_MEM(UNROM_BK2);

			map_chr_ram_extra_init(0x2000 * 4);

			// gestione mapper mirroring mapper 30
			if ((info.format == iNES_1_0) || (info.format == iNES_1_0)) {
				BYTE mirroring = (ines.flags[FL6] & 0x01) | ((ines.flags[FL6] & 0x08) >> 2);

				switch (mirroring) {
					case 0:
						mirroring_H();
						break;
					case 1:
						mirroring_V();
						break;
					case 2:
						mirroring_SCR0();
						break;
					case 3:
						// 4-Screen, cartridge VRAM
						ntbl.bank_1k[0] = &chr.extra.data[0x7000];
						ntbl.bank_1k[1] = &chr.extra.data[0x7400];
						ntbl.bank_1k[2] = &chr.extra.data[0x7800];
						ntbl.bank_1k[3] = &chr.extra.data[0x7C00];
						break;
				}
			}

			break;
	}
}

void extcl_cpu_wr_mem_UxROM(WORD address, BYTE value) {
	if (info.mapper.submapper == UXROM) {
		/* bus conflict */
		value &= prg_rom_rd(address);
	}

	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Unl1xROM(WORD address, BYTE value) {
	/* bus conflict */
	value = (value & prg_rom_rd(address)) >> 2;

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UNROM_180(UNUSED(WORD address), BYTE value) {
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UnlROM(UNUSED(WORD address), BYTE value) {
	control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UNROM_BK2(UNUSED(WORD address), BYTE value) {
	if (value & 0x80) {
		mirroring_SCR1();
	}

	{
		DBWORD bank = ((value & 0x60) >> 5) << 13;

		chr.bank_1k[0] = &chr.extra.data[bank];
		chr.bank_1k[1] = &chr.extra.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.extra.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.extra.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.extra.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.extra.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.extra.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.extra.data[bank | 0x1C00];
	}

	value &= 0x1F;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}
