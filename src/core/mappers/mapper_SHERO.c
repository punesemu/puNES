/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

INLINE static void shero_update_chr(void);

#define shero_swap_chr_1k(a, b)\
	chr1k = shero.chr_map[b];\
	shero.chr_map[b] = shero.chr_map[a];\
	shero.chr_map[a] = chr1k;
#define shero_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		WORD chr1k;\
		shero_swap_chr_1k(0, 4);\
		shero_swap_chr_1k(1, 5);\
		shero_swap_chr_1k(2, 6);\
		shero_swap_chr_1k(3, 7);\
	}
#define shero_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			value &= 0xFE;\
			shero.chr_map[mmc3.chr_rom_cfg] = value;\
			shero.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			value &= 0xFE;\
			shero.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			shero.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			shero.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			shero.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			shero.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			shero.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
	}

struct _shero {
	BYTE reg;
	WORD chr_map[8];
} shero;
struct _sherotmp {
	BYTE reset;
} sherotmp;

void map_init_SHERO(void) {
	EXTCL_CPU_WR_MEM(SHERO);
	EXTCL_CPU_RD_MEM(SHERO);
	EXTCL_SAVE_MAPPER(SHERO);
	EXTCL_WR_CHR(SHERO);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&shero;
	mapper.internal_struct_size[0] = sizeof(shero);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&shero, 0x00, sizeof(shero));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	if (info.reset >= HARD) {
		sherotmp.reset = 0;
	} else if (info.reset == RESET) {
		sherotmp.reset ^= 0xFF;
	}

	map_chr_ram_extra_init(0x2000);

	{
		BYTE i;

		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			shero.chr_map[i] = i;
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_SHERO(WORD address, BYTE value) {
	if (address == 0x4100) {
		shero.reg = value;
		shero_update_chr();
		return;
	}

	if (address >= 0x8000) {
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xF001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				shero_8000()
				shero_update_chr();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				shero_8001()
				shero_update_chr();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}
}
BYTE extcl_cpu_rd_mem_SHERO(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address == 0x4100) {
		return (sherotmp.reset);
	}
	return (openbus);
}
BYTE extcl_save_mapper_SHERO(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, shero.reg);
	save_slot_ele(mode, slot, shero.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		shero_update_chr();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_SHERO(WORD address, BYTE value) {
	chr.extra.data[address] = value;
}

INLINE static void shero_update_chr(void) {
	if (shero.reg & 0x40) {
		chr.bank_1k[0] = &chr.extra.data[0x0000];
		chr.bank_1k[1] = &chr.extra.data[0x0400];
		chr.bank_1k[2] = &chr.extra.data[0x0800];
		chr.bank_1k[3] = &chr.extra.data[0x0C00];
		chr.bank_1k[4] = &chr.extra.data[0x1000];
		chr.bank_1k[5] = &chr.extra.data[0x1400];
		chr.bank_1k[6] = &chr.extra.data[0x1800];
		chr.bank_1k[7] = &chr.extra.data[0x1C00];
	} else {
		WORD value;

		value = ((shero.reg & 0x08) << 5) | (shero.chr_map[0] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[0] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x08) << 5) | (shero.chr_map[1] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[1] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x04) << 6) | (shero.chr_map[2] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[2] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x04) << 6) | (shero.chr_map[3] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[3] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x01) << 8) | (shero.chr_map[4] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[4] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x01) << 8) | (shero.chr_map[5] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[5] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x02) << 7) | (shero.chr_map[6] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[6] = chr_chip_byte_pnt(0, value << 10);

		value = ((shero.reg & 0x02) << 7) | (shero.chr_map[7] & 0xFF);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[7] = chr_chip_byte_pnt(0, value << 10);
	}
}
