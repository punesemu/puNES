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
#include "cpu.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void h2288_update_prg(void);
INLINE static void h2288_update_chr(void);

#define h2288_chr_1k(vl) value = vl 
#define h2288_prg_8k(vl) value = (vl & 0x3F)
#define h2288_swap_chr_1k(a, b)\
	chr1k = h2288.chr_map[b];\
	h2288.chr_map[b] = h2288.chr_map[a];\
	h2288.chr_map[a] = chr1k
#define h2288_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		h2288_swap_chr_1k(0, 4);\
		h2288_swap_chr_1k(1, 5);\
		h2288_swap_chr_1k(2, 6);\
		h2288_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = h2288.prg_map[0];\
		mapper.rom_map_to[0] = h2288.prg_map[2];\
		h2288.prg_map[0] = mapper.rom_map_to[0];\
		h2288.prg_map[2] = mapper.rom_map_to[2];\
		h2288.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define h2288_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			h2288.chr_map[mmc3.chr_rom_cfg] = value;\
			h2288.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			h2288.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			h2288.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			h2288.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			h2288.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			h2288.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			h2288.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			h2288.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			h2288.prg_map[1] = value;\
			break;\
	}

struct _h2288 {
	BYTE reg[2];
	WORD prg_map[4];
	WORD chr_map[8];
} h2288;

void map_init_H2288(void) {
	EXTCL_CPU_WR_MEM(H2288);
	EXTCL_SAVE_MAPPER(H2288);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&h2288;
	mapper.internal_struct_size[0] = sizeof(h2288);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&h2288, 0x00, sizeof(h2288));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				h2288.prg_map[i] = mapper.rom_map_to[i];
			}
			h2288.chr_map[i] = i;
		}
	}

	h2288_update_prg();
	h2288_update_chr();

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_H2288(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xE001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, (value & 0xC0) | vlu114[value & 7]);
				h2288_8000()
				h2288_update_prg();
				h2288_update_chr();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				h2288_8001()
				h2288_update_prg();
				h2288_update_chr();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (address & 0x0800) {
			if (address & 0x0001) {
				h2288.reg[1] = value;
			} else {
				h2288.reg[0] = value;
				h2288_update_prg();
			}
		}
	}
}
BYTE extcl_save_mapper_H2288(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, h2288.reg);
	save_slot_ele(mode, slot, h2288.prg_map);
	save_slot_ele(mode, slot, h2288.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void h2288_update_prg(void) {
	BYTE value;

	if (h2288.reg[0] & 0x40) {
		value = (h2288.reg[0] & 0x05) | ((h2288.reg[0] & 0x08) >> 2) | ((h2288.reg[0] & 0x20) >> 2);
		if (h2288.reg[0] & 0x02) {
			value = value >> 1;
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
	} else {
		h2288_prg_8k(h2288.prg_map[0]);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		h2288_prg_8k(h2288.prg_map[1]);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		h2288_prg_8k(h2288.prg_map[2]);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		h2288_prg_8k(h2288.prg_map[3]);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void h2288_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		h2288_chr_1k(h2288.chr_map[i]);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
