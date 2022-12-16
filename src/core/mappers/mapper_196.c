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

INLINE static void m196_update_prg(void);

#define m196_8000()\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = m196.prg_map[0];\
		mapper.rom_map_to[0] = m196.prg_map[2];\
		m196.prg_map[0] = mapper.rom_map_to[0];\
		m196.prg_map[2] = mapper.rom_map_to[2];\
		m196.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom.max.banks_8k_before_last;\
	}
#define m196_8001()\
	switch (mmc3.bank_to_update) {\
		case 6:\
			m196.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			m196.prg_map[1] = value;\
			break;\
	}

struct _m196 {
	BYTE reg[2];
	WORD prg_map[4];
} m196;

void map_init_196(void) {
	EXTCL_CPU_WR_MEM(196);
	EXTCL_SAVE_MAPPER(196);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m196;
	mapper.internal_struct_size[0] = sizeof(m196);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m196, 0x00, sizeof(m196));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 4; i++) {
			m196.prg_map[i] = mapper.rom_map_to[i];
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_196(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;

		if (address >= 0xC000) {
			address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01);
		} else {
			address = (address & 0xFFFE) | ((address >> 2) & 0x01) | ((address >> 3) & 0x01)
				| ((address >> 1) & 0x01);
		}

		switch (address & 0xF001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				m196_8000()
				m196_update_prg();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				m196_8001()
				m196_update_prg();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		m196.reg[0] = 1;
		m196.reg[1] = (value & 0x0F) | (value >> 4);
		m196_update_prg();
	}
}
BYTE extcl_save_mapper_196(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m196.reg);
	save_slot_ele(mode, slot, m196.prg_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void m196_update_prg(void) {
	WORD value;

	if (m196.reg[0]) {
		value = m196.reg[1];
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		value = m196.prg_map[0];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = m196.prg_map[1];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = m196.prg_map[2];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = m196.prg_map[3];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
