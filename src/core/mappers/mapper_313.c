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

void prg_swap_313(WORD address, WORD value);
void chr_swap_313(WORD address, WORD value);

struct _m313 {
	BYTE reg;

	// da non salvare
	struct _m313_prg {
		WORD outer[2];
		WORD mask[2];
	} prg;
	struct _m313_chr {
		WORD outer;
		WORD mask;
	} chr;
} m313;

void map_init_313(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(MMC3);
	EXTCL_SAVE_MAPPER(313);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m313;
	mapper.internal_struct_size[0] = sizeof(m313);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();
	MMC3_prg_swap = prg_swap_313;
	MMC3_chr_swap = chr_swap_313;

	if (info.reset == RESET) {
		m313.reg = (m313.reg + 1) & 0x03;
	} else if (info.reset >= HARD) {
		memset(&m313, 0x00, sizeof(m313));
	}

	switch (info.mapper.submapper) {
		default:
		case 0:
			m313.prg.outer[0] = m313.prg.outer[1] = 4;
			m313.prg.mask[0] = m313.prg.mask[1] = 0x0F;
			m313.chr.outer = 7;
			m313.chr.mask = 0x7F;
			break;
		case 1:
			m313.prg.outer[0] = m313.prg.outer[1] = 5;
			m313.prg.mask[0] = m313.prg.mask[1] = 0x1F;
			m313.chr.outer = 7;
			m313.chr.mask = 0x7F;
			break;
		case 2:
			m313.prg.outer[0] = m313.prg.outer[1] = 4;
			m313.prg.mask[0] = m313.prg.mask[1] = 0x0F;
			m313.chr.outer = 8;
			m313.chr.mask = 0xFF;
			break;
		case 3:
			m313.prg.outer[0] = m313.prg.outer[1] = 5;
			m313.prg.mask[0] = m313.prg.mask[1] = 0x1F;
			m313.chr.outer = 8;
			m313.chr.mask = 0xFF;
			break;
		case 4:
			m313.prg.outer[0] = 5;
			m313.prg.mask[0] = 0x1F;
			m313.prg.outer[1] = 4;
			m313.prg.mask[1] = 0x0F;
			m313.chr.outer = 7;
			m313.chr.mask = 0x7F;
			break;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
BYTE extcl_save_mapper_313(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m313.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_313(WORD address, WORD value) {
	WORD base = m313.reg << ((m313.reg == 0) ? m313.prg.outer[0] : m313.prg.outer[1]);
	WORD mask = (m313.reg == 0) ? m313.prg.mask[0] : m313.prg.mask[1];

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_313(WORD address, WORD value) {
	WORD base = m313.reg << m313.chr.outer;
	WORD mask = m313.chr.mask;

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
