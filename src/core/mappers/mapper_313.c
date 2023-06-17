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
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_mmc3_313(WORD address, WORD value);
void chr_swap_mmc3_313(WORD address, WORD value);

struct _m313 {
	BYTE reg;
} m313;
struct _m313tmp {
	struct _m313tmp_prg {
		WORD outer[2];
		WORD mask[2];
	} prg;
	struct _m313tmp_chr {
		WORD outer;
		WORD mask;
	} chr;
} m313tmp;

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

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_313;
	MMC3_chr_swap = chr_swap_mmc3_313;

	if (info.reset == RESET) {
		m313.reg = (m313.reg + 1) & 0x03;
	} else if (info.reset >= HARD) {
		memset(&m313, 0x00, sizeof(m313));
	}

	switch (info.mapper.submapper) {
		default:
		case 0:
			m313tmp.prg.outer[0] = m313tmp.prg.outer[1] = 4;
			m313tmp.prg.mask[0] = m313tmp.prg.mask[1] = 0x0F;
			m313tmp.chr.outer = 7;
			m313tmp.chr.mask = 0x7F;
			break;
		case 1:
			m313tmp.prg.outer[0] = m313tmp.prg.outer[1] = 5;
			m313tmp.prg.mask[0] = m313tmp.prg.mask[1] = 0x1F;
			m313tmp.chr.outer = 7;
			m313tmp.chr.mask = 0x7F;
			break;
		case 2:
			m313tmp.prg.outer[0] = m313tmp.prg.outer[1] = 4;
			m313tmp.prg.mask[0] = m313tmp.prg.mask[1] = 0x0F;
			m313tmp.chr.outer = 8;
			m313tmp.chr.mask = 0xFF;
			break;
		case 3:
			m313tmp.prg.outer[0] = m313tmp.prg.outer[1] = 5;
			m313tmp.prg.mask[0] = m313tmp.prg.mask[1] = 0x1F;
			m313tmp.chr.outer = 8;
			m313tmp.chr.mask = 0xFF;
			break;
		case 4:
			m313tmp.prg.outer[0] = 5;
			m313tmp.prg.mask[0] = 0x1F;
			m313tmp.prg.outer[1] = 4;
			m313tmp.prg.mask[1] = 0x0F;
			m313tmp.chr.outer = 7;
			m313tmp.chr.mask = 0x7F;
			break;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
BYTE extcl_save_mapper_313(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m313.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_313(WORD address, WORD value) {
	WORD base = m313.reg << ((m313.reg == 0) ? m313tmp.prg.outer[0] : m313tmp.prg.outer[1]);
	WORD mask = (m313.reg == 0) ? m313tmp.prg.mask[0] : m313tmp.prg.mask[1];

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_313(WORD address, WORD value) {
	WORD base = m313.reg << m313tmp.chr.outer;
	WORD mask = m313tmp.chr.mask;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
