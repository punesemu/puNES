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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

static void INLINE malisb_update_prg(void);
static void INLINE malisb_update_chr(void);

#define malisb_prg_8k(vl)\
	(vl & 0x03) | ((vl >> 1) & 0x04) | ((vl << 1) & 0x08)
#define malisb_chr_1k(vl)\
	(vl & 0xDD) | ((vl >> 4) & 0x02) | ((vl << 4) & 0x20)
#define malisb_swap_chr_1k(a, b)\
	chr1k = malisb.chr_map[b];\
	malisb.chr_map[b] = malisb.chr_map[a];\
	malisb.chr_map[a] = chr1k
#define malisb_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		WORD chr1k;\
		malisb_swap_chr_1k(0, 4);\
		malisb_swap_chr_1k(1, 5);\
		malisb_swap_chr_1k(2, 6);\
		malisb_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = malisb.prg_map[0];\
		mapper.rom_map_to[0] = malisb.prg_map[2];\
		malisb.prg_map[0] = mapper.rom_map_to[0];\
		malisb.prg_map[2] = mapper.rom_map_to[2];\
		malisb.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define malisb_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			value &= 0xFE;\
			malisb.chr_map[mmc3.chr_rom_cfg] = value;\
			malisb.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			value &= 0xFE;\
			malisb.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			malisb.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			malisb.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			malisb.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			malisb.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			malisb.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			malisb.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			malisb.prg_map[1] = value;\
			break;\
	}

void map_init_MALISB(void) {
	EXTCL_CPU_WR_MEM(MALISB);
	EXTCL_SAVE_MAPPER(MALISB);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &malisb;
	mapper.internal_struct_size[0] = sizeof(malisb);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&malisb, 0x00, sizeof(malisb));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				malisb.prg_map[i] = mapper.rom_map_to[i];
			}
			malisb.chr_map[i] = i;
		}

		malisb_update_prg();
		malisb_update_chr();
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_MALISB(WORD address, BYTE value) {
	BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
	BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

	if (address < 0xC000) {
		address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01);
	} else {
		address = (address & 0xFFFE) | ((address >> 3) & 0x01);
	}

	switch (address & 0xE001) {
		case 0x8000:
			extcl_cpu_wr_mem_MMC3(address, value);
			malisb_8000()
			malisb_update_prg();
			malisb_update_chr();
			return;
		case 0x8001:
			extcl_cpu_wr_mem_MMC3(address, value);
			malisb_8001()
			malisb_update_prg();
			malisb_update_chr();
			return;
		default:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_save_mapper_MALISB(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, malisb.prg_map);
	save_slot_ele(mode, slot, malisb.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

static void INLINE malisb_update_prg(void) {
	WORD value;

	value = malisb_prg_8k(malisb.prg_map[0]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);

	value = malisb_prg_8k(malisb.prg_map[1]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);

	value = malisb_prg_8k(malisb.prg_map[2]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 2, value);

	value = malisb_prg_8k(malisb.prg_map[3]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 3, value);

	map_prg_rom_8k_update();
}
static void INLINE malisb_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		value = malisb_chr_1k(malisb.chr_map[i]) | i;
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
