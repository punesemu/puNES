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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void m197_update_chr(void);

#define m197_swap_chr_1k(a, b)\
	chr1k = m197.chr_map[b];\
	m197.chr_map[b] = m197.chr_map[a];\
	m197.chr_map[a] = chr1k
#define m197_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		WORD chr1k;\
		m197_swap_chr_1k(0, 4);\
		m197_swap_chr_1k(1, 5);\
		m197_swap_chr_1k(2, 6);\
		m197_swap_chr_1k(3, 7);\
	}
#define m197_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			m197.chr_map[mmc3.chr_rom_cfg] = value;\
			m197.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			m197.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			m197.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			m197.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			m197.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			m197.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			m197.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
	}

struct _m197 {
	WORD chr_map[8];
} m197;

void map_init_197(void) {
	EXTCL_CPU_WR_MEM(197);
	EXTCL_SAVE_MAPPER(197);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m197;
	mapper.internal_struct_size[0] = sizeof(m197);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m197, 0x00, sizeof(m197));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			m197.chr_map[i] = i;
		}
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_197(WORD address, BYTE value) {
	BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

	switch (address & 0xF001) {
		case 0x8000:
			extcl_cpu_wr_mem_MMC3(address, value);
			m197_8000()
			m197_update_chr();
			return;
		case 0x8001:
			extcl_cpu_wr_mem_MMC3(address, value);
			m197_8001()
			m197_update_chr();
			return;
		default:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_save_mapper_197(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m197.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void m197_update_chr(void) {
	WORD value;
	DBWORD bank;

	value = m197.chr_map[0] >> 1;
	control_bank(info.chr.rom.max.banks_4k)
	bank = value << 12;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);

	value = m197.chr_map[4];
	control_bank(info.chr.rom.max.banks_2k)
	bank = value << 11;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);

	value = m197.chr_map[5];
	control_bank(info.chr.rom.max.banks_2k)
	bank = value << 11;
	chr.bank_1k[6] = chr_pnt(bank);
	chr.bank_1k[7] = chr_pnt(bank | 0x0400);
}
