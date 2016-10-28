/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

static void INLINE sl1632_update_mmc3(void);
static void INLINE sl1632_update_chr_mmc3(void);
static void INLINE sl1632_update(void);

#define sl1632_change_page_chr_1k_mmc3(ind)\
	sl1632.mmc3.chr_map[ind] = value | (sl1632.mmc3.chr_map[ind] & 0xFF)
#define sl1632_swap_chr_1k_mmc3(a, b)\
	chr1k = sl1632.mmc3.chr_map[b];\
	sl1632.mmc3.chr_map[b] = sl1632.mmc3.chr_map[a];\
	sl1632.mmc3.chr_map[a] = chr1k;
#define sl1632_8000_mmc3()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		WORD chr1k;\
		sl1632_swap_chr_1k_mmc3(0, 4);\
		sl1632_swap_chr_1k_mmc3(1, 5);\
		sl1632_swap_chr_1k_mmc3(2, 6);\
		sl1632_swap_chr_1k_mmc3(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = sl1632.mmc3.prg_map[0];\
		mapper.rom_map_to[0] = sl1632.mmc3.prg_map[2];\
		sl1632.mmc3.prg_map[0] = mapper.rom_map_to[0];\
		sl1632.mmc3.prg_map[2] = mapper.rom_map_to[2];\
		sl1632.mmc3.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define sl1632_8001_mmc3()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			value &= 0xFE;\
			sl1632.mmc3.chr_map[mmc3.chr_rom_cfg] = value;\
			sl1632.mmc3.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			value &= 0xFE;\
			sl1632.mmc3.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			sl1632.mmc3.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			sl1632.mmc3.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			sl1632.mmc3.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			sl1632.mmc3.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			sl1632.mmc3.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			sl1632.mmc3.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			sl1632.mmc3.prg_map[1] = value;\
			break;\
	}
#define sl1632_chr_1k_ns(i)\
	tmp = (sl1632.chr_map[i] & 0xF0) | (value & 0x0F);\
	_control_bank(tmp, info.chr.rom[0].max.banks_1k)\
	chr.bank_1k[i] = chr_chip_byte_pnt(0, tmp << 10);\
	sl1632.chr_map[i] = tmp
#define sl1632_chr_1k_4s(i)\
	tmp = (sl1632.chr_map[i] & (0xF0 >> 4)) | ((value & 0x0F) << 4);\
	_control_bank(tmp, info.chr.rom[0].max.banks_1k)\
	chr.bank_1k[i] = chr_chip_byte_pnt(0, tmp << 10);\
	sl1632.chr_map[i] = tmp
#define sl1632_mirroring(mr)\
	if (mr) {\
		mirroring_H();\
	} else {\
		mirroring_V();\
	}

void map_init_SL1632(void) {
	EXTCL_CPU_WR_MEM(SL1632);
	EXTCL_SAVE_MAPPER(SL1632);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &sl1632;
	mapper.internal_struct_size[0] = sizeof(sl1632);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&sl1632, 0x00, sizeof(sl1632));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				sl1632.mmc3.prg_map[i] = mapper.rom_map_to[i];
			}
			sl1632.mmc3.chr_map[i] = sl1632.chr_map[i] = i;
		}
		sl1632.prg_map[0] = mapper.rom_map_to[0];
		sl1632.prg_map[1] = mapper.rom_map_to[1];
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_SL1632(WORD address, BYTE value) {
	if (address < 0x4100) {
		return;
	}

	if (address == 0xA131) {
		if (sl1632.mode != value) {
			sl1632.mode = value;
			if (value & 0x02) {
				sl1632_update_mmc3();
			} else {
				sl1632_update();
			}
		}
	}

	if (sl1632.mode & 0x02) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xF001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				sl1632_8000_mmc3()
				sl1632_update_chr_mmc3();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				sl1632_8001_mmc3()
				sl1632_update_chr_mmc3();
				return;
			case 0xA000:
				extcl_cpu_wr_mem_MMC3(address, value);
				sl1632.mmc3.mirroring = value & 0x01;
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	} else {
		WORD tmp;

		switch (address & 0xF003) {
			case 0x8000:
				control_bank(info.prg.rom[0].max.banks_8k)
				map_prg_rom_8k(1, 0, value);
				map_prg_rom_8k_update();
				sl1632.prg_map[0] = value;
				return;
			case 0x9000:
				sl1632.mirroring = value & 0x01;
				sl1632_mirroring(sl1632.mirroring)
				return;
			case 0xA000:
				control_bank(info.prg.rom[0].max.banks_8k)
				map_prg_rom_8k(1, 1, value);
				map_prg_rom_8k_update();
				sl1632.prg_map[1] = value;
				return;
			case 0xB000:
				sl1632_chr_1k_ns(0);
				return;
			case 0xB001:
				sl1632_chr_1k_4s(0);
				return;
			case 0xB002:
				sl1632_chr_1k_ns(1);
				return;
			case 0xB003:
				sl1632_chr_1k_4s(1);
				return;
			case 0xC000:
				sl1632_chr_1k_ns(2);
				return;
			case 0xC001:
				sl1632_chr_1k_4s(2);
				return;
			case 0xC002:
				sl1632_chr_1k_ns(3);
				return;
			case 0xC003:
				sl1632_chr_1k_4s(3);
				return;
			case 0xD000:
				sl1632_chr_1k_ns(4);
				return;
			case 0xD001:
				sl1632_chr_1k_4s(4);
				return;
			case 0xD002:
				sl1632_chr_1k_ns(5);
				return;
			case 0xD003:
				sl1632_chr_1k_4s(5);
				return;
			case 0xE000:
				sl1632_chr_1k_ns(6);
				return;
			case 0xE001:
				sl1632_chr_1k_4s(6);
				return;
			case 0xE002:
				sl1632_chr_1k_ns(7);
				return;
			case 0xE003:
				sl1632_chr_1k_4s(7);
				return;
		}
	}
}
BYTE extcl_save_mapper_SL1632(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, sl1632.mode);
	save_slot_ele(mode, slot, sl1632.mirroring);
	save_slot_ele(mode, slot, sl1632.prg_map);
	save_slot_ele(mode, slot, sl1632.chr_map);
	save_slot_ele(mode, slot, sl1632.mmc3.mirroring);
	save_slot_ele(mode, slot, sl1632.mmc3.prg_map);
	save_slot_ele(mode, slot, sl1632.mmc3.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

static void INLINE sl1632_update_mmc3(void) {
	WORD value;

	value = sl1632.mmc3.prg_map[0];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);
	value = sl1632.mmc3.prg_map[1];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);
	value = sl1632.mmc3.prg_map[2];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 2, value);
	value = sl1632.mmc3.prg_map[3];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 3, value);
	map_prg_rom_8k_update();

	sl1632_update_chr_mmc3();

	sl1632_mirroring(sl1632.mmc3.mirroring)
}
static void INLINE sl1632_update_chr_mmc3(void) {
	BYTE i;
	WORD value;

	value = ((sl1632.mode & 0x08) << 5);
	sl1632_change_page_chr_1k_mmc3(mmc3.chr_rom_cfg | 0x00);
	sl1632_change_page_chr_1k_mmc3(mmc3.chr_rom_cfg | 0x01);
	sl1632_change_page_chr_1k_mmc3(mmc3.chr_rom_cfg | 0x02);
	sl1632_change_page_chr_1k_mmc3(mmc3.chr_rom_cfg | 0x03);
	value = (sl1632.mode & 0x20) << 3;
	sl1632_change_page_chr_1k_mmc3((mmc3.chr_rom_cfg ^ 0x04) | 0x00);
	sl1632_change_page_chr_1k_mmc3((mmc3.chr_rom_cfg ^ 0x04) | 0x01);
	value = (sl1632.mode & 0x80) << 1;
	sl1632_change_page_chr_1k_mmc3((mmc3.chr_rom_cfg ^ 0x04) | 0x02);
	sl1632_change_page_chr_1k_mmc3((mmc3.chr_rom_cfg ^ 0x04) | 0x03);

	for (i = 0; i < 8; i++) {
		value = sl1632.mmc3.chr_map[i];
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
static void INLINE sl1632_update(void) {
	BYTE i, value;

	value = sl1632.prg_map[0];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);
	value = sl1632.prg_map[1];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);
	value = 0xFE;
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 2, value);
	value = 0xFF;
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 3, value);
	map_prg_rom_8k_update();

	for (i = 0; i < 8; i++) {
		value = sl1632.chr_map[i];
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}

	sl1632_mirroring(sl1632.mirroring)
}
