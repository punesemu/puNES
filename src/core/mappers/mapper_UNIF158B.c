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

static void INLINE unif158b_update_prg(void);

#define unif158b_8000()\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = unif158b.prg_map[0];\
		mapper.rom_map_to[0] = unif158b.prg_map[2];\
		unif158b.prg_map[0] = mapper.rom_map_to[0];\
		unif158b.prg_map[2] = mapper.rom_map_to[2];\
		unif158b.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define unif158b_8001()\
	switch (mmc3.bank_to_update) {\
		case 6:\
			unif158b.prg_map[mmc3.prg_rom_cfg] = value & 0x0F;\
			break;\
		case 7:\
			unif158b.prg_map[1] = value & 0x0F;\
			break;\
	}

static const BYTE unif158b_vlu[8] = { 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0F, 0x00 };

void map_init_UNIF158B(void) {
	EXTCL_CPU_WR_MEM(UNIF158B);
	EXTCL_CPU_RD_MEM(UNIF158B);
	EXTCL_SAVE_MAPPER(UNIF158B);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &unif158b;
	mapper.internal_struct_size[0] = sizeof(unif158b);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&unif158b, 0x00, sizeof(unif158b));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();

		for (i = 0; i < 4; i++) {
			unif158b.prg_map[i] = mapper.rom_map_to[i];
		}
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_UNIF158B(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;

		switch (address) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				unif158b_8000()
				unif158b_update_prg();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				unif158b_8001()
				unif158b_update_prg();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		unif158b.reg[address & 0x07] = value;
		unif158b_update_prg();
	}
}
BYTE extcl_cpu_rd_mem_UNIF158B(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return (openbus);
	}

	return (unif158b_vlu[address & 0x07]);
}
BYTE extcl_save_mapper_UNIF158B(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unif158b.reg);
	save_slot_ele(mode, slot, unif158b.prg_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

static void INLINE unif158b_update_prg(void) {
	WORD value;

	if (unif158b.reg[0] & 0x80) {
		value = unif158b.reg[0] & 0x07;

		if (unif158b.reg[0] & 0x20) {
			value = value >> 1;
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
	} else {
		value = unif158b.prg_map[0];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = unif158b.prg_map[1];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = unif158b.prg_map[2];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = unif158b.prg_map[3];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
