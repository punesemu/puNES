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

INLINE static void dragonfighter_update_prg(void);
INLINE static void dragonfighter_update_chr(void);

#define dragonfighter_swap_chr_1k(a, b)\
	chr1k = dragonfighter.chr_map[b];\
	dragonfighter.chr_map[b] = dragonfighter.chr_map[a];\
	dragonfighter.chr_map[a] = chr1k
#define dragonfighter_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		dragonfighter_swap_chr_1k(0, 4);\
		dragonfighter_swap_chr_1k(1, 5);\
		dragonfighter_swap_chr_1k(2, 6);\
		dragonfighter_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = dragonfighter.prg_map[0];\
		mapper.rom_map_to[0] = dragonfighter.prg_map[2];\
		dragonfighter.prg_map[0] = mapper.rom_map_to[0];\
		dragonfighter.prg_map[2] = mapper.rom_map_to[2];\
		dragonfighter.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define dragonfighter_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			dragonfighter.chr_map[mmc3.chr_rom_cfg] = value;\
			dragonfighter.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			dragonfighter.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			dragonfighter.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			dragonfighter.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			dragonfighter.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			dragonfighter.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			dragonfighter.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			dragonfighter.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			dragonfighter.prg_map[1] = value;\
			break;\
	}

struct _dragonfighter {
	BYTE reg[3];
	WORD prg_map[4];
	WORD chr_map[8];
} dragonfighter;

void map_init_DRAGONFIGHTER(void) {
	EXTCL_CPU_WR_MEM(DRAGONFIGHTER);
	EXTCL_CPU_RD_MEM(DRAGONFIGHTER);
	EXTCL_SAVE_MAPPER(DRAGONFIGHTER);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&dragonfighter;
	mapper.internal_struct_size[0] = sizeof(dragonfighter);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&dragonfighter, 0x00, sizeof(dragonfighter));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				dragonfighter.prg_map[i] = mapper.rom_map_to[i];
			}
			dragonfighter.chr_map[i] = i;
		}
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_DRAGONFIGHTER(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xE001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				dragonfighter_8000()
				dragonfighter_update_prg();
				dragonfighter_update_chr();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				dragonfighter_8001()
				dragonfighter_update_prg();
				dragonfighter_update_chr();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if (((address & 0xF000) == 0x6000) && !(address & 0x0001)) {
		dragonfighter.reg[0] = value;
		dragonfighter_update_prg();
	}
}
BYTE extcl_cpu_rd_mem_DRAGONFIGHTER(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (((address & 0xF000) == 0x6000) && !(address & 0x0001)) {
		if ((dragonfighter.reg[0] & 0xE0) == 0xC0) {
			dragonfighter.reg[1] = mmcpu.ram[0x06A];
		} else {
			dragonfighter.reg[2] = mmcpu.ram[0x0FF];
		}
		dragonfighter_update_chr();
	}
	return (openbus);
}
BYTE extcl_save_mapper_DRAGONFIGHTER(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, dragonfighter.reg);
	save_slot_ele(mode, slot, dragonfighter.prg_map);
	save_slot_ele(mode, slot, dragonfighter.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void dragonfighter_update_prg(void) {
	BYTE value;

	value = dragonfighter.reg[0] & 0x1F;
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);

	map_prg_rom_8k_update();
}
INLINE static void dragonfighter_update_chr(void) {
	BYTE value;
	SDBWORD bank;

	value = (dragonfighter.chr_map[0] >> 1) ^ dragonfighter.reg[1];
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x400);

	value = (dragonfighter.chr_map[2] >> 1) | ((dragonfighter.reg[2] & 0x40) << 1);
	control_bank(info.chr.rom[0].max.banks_2k)
	bank = value << 11;
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x400);

	value = dragonfighter.reg[2] & 0x3F;
	control_bank(info.chr.rom[0].max.banks_4k)
	bank = value << 12;
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0xC00);
}
