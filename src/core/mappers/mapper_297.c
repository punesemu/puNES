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

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_297(void);
INLINE static void chr_fix_297(void);
INLINE static void mirroring_fix_297(void);

struct _m297 {
	BYTE reg[2];
} m297;

void map_init_297(void) {
	info.mapper.submapper = MAP297;
	map_init_MMC1();

	EXTCL_AFTER_MAPPER_INIT(297);
	EXTCL_CPU_WR_MEM(297);
	EXTCL_SAVE_MAPPER(297);
	mapper.internal_struct[0] = (BYTE *)&m297;
	mapper.internal_struct_size[0] = sizeof(m297);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	if (info.reset >= HARD) {
		memset(&m297, 0x00, sizeof(m297));
	}

	mmc1.prg_mask = 0x07;
	mmc1.prg_upper = 0x08;
	mmc1.chr_upper = 0x20;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_297(void) {
	prg_fix_297();
	chr_fix_297();
	mirroring_fix_297();
}
void extcl_cpu_wr_mem_297(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
		case 0x5000:
			if (address & 0x0100) {
				m297.reg[0] = value;
				prg_fix_297();
				chr_fix_297();
				mirroring_fix_297();
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m297.reg[0] & 0x01) {
				extcl_cpu_wr_mem_MMC1(address, value);
			} else {
				m297.reg[1] = value;
				prg_fix_297();
				chr_fix_297();
				mirroring_fix_297();
			}
			break;
	}
}
BYTE extcl_save_mapper_297(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m297.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_297(void) {
	WORD bank[2];

	if (m297.reg[0] & 0x01) {
		bank[0] = mmc1.prg_upper | (mmc1.prg0 & mmc1.prg_mask);
		bank[1] = mmc1.prg_upper | mmc1.prg_mask;
	} else {
		bank[0] = ((m297.reg[0] & 0x02) << 1) | ((m297.reg[1] & 0x30) >> 4);
		bank[1] = ((m297.reg[0] & 0x02) << 1) | 0x03;
	}

	_control_bank(bank[0], info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank[0]);

	_control_bank(bank[1], info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank[1]);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_297(void) {
	DBWORD bank;

	bank = m297.reg[1] & 0x0F;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_297(void) {
	mirroring_V();
}
