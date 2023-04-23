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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_SL1632(WORD address, WORD value);
void chr_swap_SL1632(WORD address, WORD value);

INLINE static void sl1632_update(void);

#define sl1632_chr_1k_ns(i)\
	tmp = (sl1632.chr_map[i] & 0xF0) | (value & 0x0F);\
	_control_bank(tmp, info.chr.rom.max.banks_1k)\
	chr.bank_1k[i] = chr_pnt(tmp << 10);\
	sl1632.chr_map[i] = tmp
#define sl1632_chr_1k_4s(i)\
	tmp = (sl1632.chr_map[i] & (0xF0 >> 4)) | ((value & 0x0F) << 4);\
	_control_bank(tmp, info.chr.rom.max.banks_1k)\
	chr.bank_1k[i] = chr_pnt(tmp << 10);\
	sl1632.chr_map[i] = tmp
#define sl1632_mirroring(mr)\
	if (mr) {\
		mirroring_H();\
	} else {\
		mirroring_V();\
	}

struct _sl1632 {
	BYTE mode;
	BYTE mirroring;
	WORD prg_map[2];
	WORD chr_map[8];
} sl1632;

void map_init_SL1632(void) {
	EXTCL_AFTER_MAPPER_INIT(SL1632);
	EXTCL_CPU_WR_MEM(SL1632);
	EXTCL_SAVE_MAPPER(SL1632);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&sl1632;
	mapper.internal_struct_size[0] = sizeof(sl1632);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&sl1632, 0x00, sizeof(sl1632));

	init_MMC3();
	MMC3_prg_swap = prg_swap_SL1632;
	MMC3_chr_swap = chr_swap_SL1632;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_SL1632(void) {
	if (sl1632.mode & 0x02) {
		extcl_after_mapper_init_MMC3();
	} else {
		sl1632.prg_map[0] = mapper.rom_map_to[0];
		sl1632.prg_map[1] = mapper.rom_map_to[1];
		sl1632_update();
	}
}
void extcl_cpu_wr_mem_SL1632(WORD address, BYTE value) {
	if (address == 0xA131) {
		sl1632.mode = value;
		if (value & 0x02) {
			MMC3_prg_fix(mmc3.bank_to_update);
			MMC3_chr_fix(mmc3.bank_to_update);
			MMC3_mirroring_fix();
		} else {
			sl1632_update();
		}
	}

	if (sl1632.mode & 0x02) {
		extcl_cpu_wr_mem_MMC3(address, value);
	} else {
		WORD tmp = 0;

		switch (address & 0xF003) {
			case 0x8000:
				control_bank(info.prg.rom.max.banks_8k)
				map_prg_rom_8k(1, 0, value);
				map_prg_rom_8k_update();
				sl1632.prg_map[0] = value;
				return;
			case 0x9000:
				sl1632.mirroring = value & 0x01;
				sl1632_mirroring(sl1632.mirroring)
				return;
			case 0xA000:
				control_bank(info.prg.rom.max.banks_8k)
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
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_SL1632(WORD address, WORD value) {
	control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_SL1632(WORD address, WORD value) {
	const BYTE slot = address >> 10;
	WORD base = 0;
	WORD mask = 0xFF;

	switch (slot) {
		case 0:
		case 1:
		case 2:
		case 3:
			base = (sl1632.mode & 0x08) << 5;
			break;
		case 4:
		case 5:
			base = (sl1632.mode & 0x20) << 3;
			break;
		case 6:
		case 7:
			base = (sl1632.mode & 0x80) << 1;
			break;
		default:
			break;
	}

	value = base | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[slot] = chr_pnt(value << 10);
}

INLINE static void sl1632_update(void) {
	BYTE i = 0, value = 0;

	value = sl1632.prg_map[0];
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, value);
	value = sl1632.prg_map[1];
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, value);
	value = 0xFE;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, value);
	value = 0xFF;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, value);
	map_prg_rom_8k_update();

	for (i = 0; i < 8; i++) {
		value = sl1632.chr_map[i];
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[i] = chr_pnt(value << 10);
	}

	sl1632_mirroring(sl1632.mirroring)
}
