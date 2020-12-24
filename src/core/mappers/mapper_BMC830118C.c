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
#include "cpu.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void bmc830118c_update_prg(void);
INLINE static void bmc830118c_update_chr(void);

#define bmc830118c_chr_1k(vl) value = (vl & 0x7F) | ((bmc830118c.reg & 0x0C) << 5)
#define bmc830118c_prg_8k(vl) value = (vl & 0x0F) | ((bmc830118c.reg & 0x0C) << 2)
#define bmc830118c_swap_chr_1k(a, b)\
	chr1k = bmc830118c.chr_map[b];\
	bmc830118c.chr_map[b] = bmc830118c.chr_map[a];\
	bmc830118c.chr_map[a] = chr1k
#define bmc830118c_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		bmc830118c_swap_chr_1k(0, 4);\
		bmc830118c_swap_chr_1k(1, 5);\
		bmc830118c_swap_chr_1k(2, 6);\
		bmc830118c_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = bmc830118c.prg_map[0];\
		mapper.rom_map_to[0] = bmc830118c.prg_map[2];\
		bmc830118c.prg_map[0] = mapper.rom_map_to[0];\
		bmc830118c.prg_map[2] = mapper.rom_map_to[2];\
		bmc830118c.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define bmc830118c_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			bmc830118c.chr_map[mmc3.chr_rom_cfg] = value;\
			bmc830118c.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			bmc830118c.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			bmc830118c.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			bmc830118c.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			bmc830118c.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			bmc830118c.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			bmc830118c.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			bmc830118c.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			bmc830118c.prg_map[1] = value;\
			break;\
	}

struct _bmc830118c {
	BYTE reg;
	WORD prg_map[4];
	WORD chr_map[8];
} bmc830118c;

void map_init_BMC830118C(void) {
	EXTCL_CPU_WR_MEM(BMC830118C);
	EXTCL_SAVE_MAPPER(BMC830118C);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &bmc830118c;
	mapper.internal_struct_size[0] = sizeof(bmc830118c);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&bmc830118c, 0x00, sizeof(bmc830118c));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				bmc830118c.prg_map[i] = mapper.rom_map_to[i];
			}
			bmc830118c.chr_map[i] = i;
		}
	}

	bmc830118c_update_prg();
	bmc830118c_update_chr();

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_BMC830118C(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xE001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				bmc830118c_8000()
				bmc830118c_update_prg();
				bmc830118c_update_chr();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				bmc830118c_8001()
				bmc830118c_update_prg();
				bmc830118c_update_chr();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if ((address >= 0x6800) && (address <= 0x68FF)) {
		bmc830118c.reg = value;
		bmc830118c_update_prg();
		bmc830118c_update_chr();
	}
}
BYTE extcl_save_mapper_BMC830118C(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc830118c.reg);
	save_slot_ele(mode, slot, bmc830118c.prg_map);
	save_slot_ele(mode, slot, bmc830118c.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void bmc830118c_update_prg(void) {
	BYTE value;

	bmc830118c_prg_8k(bmc830118c.prg_map[0]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);

	bmc830118c_prg_8k(bmc830118c.prg_map[1]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);

	if ((bmc830118c.reg & 0x0C) == 0x0C) {
		value = (bmc830118c.prg_map[0] & 0x0F) | 0x32;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = (bmc830118c.prg_map[1] & 0x0F) | 0x32;
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	} else {
		bmc830118c_prg_8k(bmc830118c.prg_map[2]);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		bmc830118c_prg_8k(bmc830118c.prg_map[3]);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void bmc830118c_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		bmc830118c_chr_1k(bmc830118c.chr_map[i]);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
