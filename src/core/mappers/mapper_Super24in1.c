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

INLINE static void super24in1_update_prg(void);
INLINE static void super24in1_update_chr(void);

#define super24in1_prg_value(bnk)\
	(super24in1.prg_map[bnk] & super24in1_mask[super24in1.reg[0] & 0x07]) | (super24in1.reg[1] << 1)
#define super24in1_swap_chr_1k(a, b)\
	chr1k = super24in1.chr_map[b];\
	super24in1.chr_map[b] = super24in1.chr_map[a];\
	super24in1.chr_map[a] = chr1k
#define super24in1_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		super24in1_swap_chr_1k(0, 4);\
		super24in1_swap_chr_1k(1, 5);\
		super24in1_swap_chr_1k(2, 6);\
		super24in1_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = super24in1.prg_map[0];\
		mapper.rom_map_to[0] = super24in1.prg_map[2];\
		super24in1.prg_map[0] = mapper.rom_map_to[0];\
		super24in1.prg_map[2] = mapper.rom_map_to[2];\
		super24in1.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define super24in1_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			super24in1.chr_map[mmc3.chr_rom_cfg] = value;\
			super24in1.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			super24in1.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			super24in1.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			super24in1.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			super24in1.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			super24in1.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			super24in1.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			super24in1.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			super24in1.prg_map[1] = value;\
			break;\
	}

static const BYTE super24in1_mask[8] = { 0x3F, 0x1F, 0x0F, 0x01, 0x03, 0x00, 0x00, 0x00 };

struct _super24in1 {
	BYTE reg[3];
	WORD prg_map[4];
	WORD chr_map[8];
} super24in1;

void map_init_Super24in1(void) {
	EXTCL_CPU_WR_MEM(Super24in1);
	EXTCL_SAVE_MAPPER(Super24in1);
	EXTCL_WR_CHR(Super24in1);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &super24in1;
	mapper.internal_struct_size[0] = sizeof(super24in1);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&super24in1, 0x00, sizeof(super24in1));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				super24in1.prg_map[i] = mapper.rom_map_to[i];
			}
			super24in1.chr_map[i] = i;
		}
	}

	map_chr_ram_extra_init(0x2000);

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;

	super24in1_update_prg();
	super24in1_update_chr();
}
void extcl_cpu_wr_mem_Super24in1(WORD address, BYTE value) {
	BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
	BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

	switch (address) {
		case 0x5FF0:
			super24in1.reg[0] = value;
			super24in1_update_prg();
			super24in1_update_chr();
			return;
		case 0x5FF1:
			super24in1.reg[1] = value;
			super24in1_update_prg();
			return;
		case 0x5FF2:
			super24in1.reg[2] = value;
			super24in1_update_chr();
			return;
		case 0x8000:
			extcl_cpu_wr_mem_MMC3(address, value);
			super24in1_8000()
			super24in1_update_prg();
			super24in1_update_chr();
			return;
		case 0x8001:
			extcl_cpu_wr_mem_MMC3(address, value);
			super24in1_8001()
			super24in1_update_prg();
			super24in1_update_chr();
			return;
		default:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_save_mapper_Super24in1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, super24in1.reg);
	save_slot_ele(mode, slot, super24in1.prg_map);
	save_slot_ele(mode, slot, super24in1.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_wr_chr_Super24in1(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (super24in1.reg[0] & 0x20) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

INLINE static void super24in1_update_prg(void) {
	BYTE chip;
	WORD value;

	value = super24in1_prg_value(0);
	chip = (value >> 6) & 0x0F;
	_control_bank(chip, info.prg.max_chips)
	control_bank(info.prg.rom[chip].max.banks_8k)
	map_prg_rom_8k_chip(1, 0, value, chip);

	value = super24in1_prg_value(1);
	chip = (value >> 6) & 0x0F;
	_control_bank(chip, info.prg.max_chips)
	control_bank(info.prg.rom[chip].max.banks_8k)
	map_prg_rom_8k_chip(1, 1, value, chip);

	value = super24in1_prg_value(2);
	chip = (value >> 6) & 0x0F;
	_control_bank(chip, info.prg.max_chips)
	control_bank(info.prg.rom[chip].max.banks_8k)
	map_prg_rom_8k_chip(1, 2, value, chip);

	value = super24in1_prg_value(3);
	chip = (value >> 6) & 0x0F;
	_control_bank(chip, info.prg.max_chips)
	control_bank(info.prg.rom[chip].max.banks_8k)
	map_prg_rom_8k_chip(1, 3, value, chip);

	map_prg_rom_8k_update();
}
INLINE static void super24in1_update_chr(void) {
	BYTE chip, i;
	WORD value;

	for (i = 0; i < 8; i++) {
		if (super24in1.reg[0] & 0x20) {
			value = super24in1.chr_map[i];
			control_bank(0x07)
			chr.bank_1k[i] = &chr.extra.data[i << 10];
		} else {
			value = super24in1.chr_map[i] | (super24in1.reg[2] << 3);
			chip = (value >> 9) & 0x0F;
			_control_bank(chip, info.chr.max_chips)
			control_bank(info.chr.rom[chip].max.banks_1k)
			chr.bank_1k[i] = chr_chip_byte_pnt(chip, value << 10);
		}
	}
}
