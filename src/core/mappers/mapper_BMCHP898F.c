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

enum _bmchp898f_types { BMCHP898F_0, BMCHP898F_1 };

struct _bmchp898ftmp {
	BYTE model;
	BYTE reset;
} bmchp898ftmp;

void map_init_BMCHP898F(void) {
	EXTCL_AFTER_MAPPER_INIT(BMCHP898F);
	EXTCL_CPU_WR_MEM(BMCHP898F);
	EXTCL_CPU_RD_MEM(BMCHP898F);

	if (info.reset == RESET) {
		bmchp898ftmp.reset = !bmchp898ftmp.reset;
	} else if (info.reset >= HARD) {
		bmchp898ftmp.reset = 0;
	}

	// Prima Soft 9999999-in-1 (02 8807870-3)(Unl)[U][!].nes
	if ((prg_size() == (1024 * 128)) && (info.crc32.prg == 0xC25FD362)) {
		bmchp898ftmp.model = BMCHP898F_1;
	} else {
		bmchp898ftmp.model = BMCHP898F_0;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_BMCHP898F(void) {
	extcl_cpu_wr_mem_BMCHP898F(0x6004, 0);
}
void extcl_cpu_wr_mem_BMCHP898F(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		switch (address & 0x4) {
			case 0: {
				DBWORD bank;

				value = (value & 0xF0) >> 4;
				control_bank(info.chr.rom.max.banks_8k)
				bank = value << 13;
				chr.bank_1k[0] = chr_pnt(bank);
				chr.bank_1k[1] = chr_pnt(bank | 0x0400);
				chr.bank_1k[2] = chr_pnt(bank | 0x0800);
				chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
				chr.bank_1k[4] = chr_pnt(bank | 0x1000);
				chr.bank_1k[5] = chr_pnt(bank | 0x1400);
				chr.bank_1k[6] = chr_pnt(bank | 0x1800);
				chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
				return;
			}
			case 4: {
				BYTE base, mode;

				if (bmchp898ftmp.model == BMCHP898F_1) {
					// The publicly-available UNIF ROM file of Prima Soft 9999999-in-1 has the order of the 16 KiB PRG-ROM
					// banks slightly mixed up, so that the PRG A14 mode bit operates on A16 instead of A14. To obtain the
					// correct bank order, use UNIF 16 KiB PRG banks 0, 4, 1, 5, 2, 6, 3, 7.
					base = (value & 0x38) >> 3;
					mode = (value & 0x40) >> 4;
				} else {
					base = ((value & 0x18) >> 2) | ((value & 0x20) >> 5);
					mode = (value & 0x40) >> 6;
				}

				if (value & 0x80) {
					mirroring_V();
				} else {
					mirroring_H();
				}

				value = base & ~mode;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, value);
				value = base | mode;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 2, value);
				map_prg_rom_8k_update();
				return;
			}
		}
	}
}
BYTE extcl_cpu_rd_mem_BMCHP898F(WORD address, BYTE openbus) {
	if (address == 0x5FF0) {
		return (bmchp898ftmp.reset << 6);
	}
	return (openbus);
}
