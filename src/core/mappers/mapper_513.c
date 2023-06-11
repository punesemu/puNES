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

void prg_fix_mmc3_513(void);
void chr_swap_mmc3_513(WORD address, WORD value);

struct _m513 {
	BYTE reg;
} m513;

void map_init_513(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(513);
	EXTCL_SAVE_MAPPER(513);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m513;
	mapper.internal_struct_size[0] = sizeof(m513);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m513, 0x00, sizeof(m513));

	init_MMC3();
	MMC3_prg_fix = prg_fix_mmc3_513;
	MMC3_chr_swap = chr_swap_mmc3_513;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_513(WORD address, BYTE value) {
	if ((address & 0xE001) == 0x8001) {
		switch (mmc3.bank_to_update & 0x07) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				m513.reg = value;
				extcl_cpu_wr_mem_MMC3(address, value);
				break;
			default:
				mmc3.reg[mmc3.bank_to_update & 0x07] = value;
				break;
		}
		MMC3_prg_fix();
		return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_513(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m513.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_fix_mmc3_513(void) {
	if (mmc3.bank_to_update & 0x40) {
		MMC3_prg_swap(0x8000, (~1 & 0x3F));
		MMC3_prg_swap(0xC000, (m513.reg & 0xC0) | (mmc3.reg[6] & 0x3F));
	} else {
		MMC3_prg_swap(0x8000, (m513.reg & 0xC0) | (mmc3.reg[6] & 0x3F));
		MMC3_prg_swap(0xC000, (~1 & 0x3F));
	}
	MMC3_prg_swap(0xA000, (m513.reg & 0xC0) | (mmc3.reg[7] & 0x3F));
	MMC3_prg_swap(0xE000, (~0 & 0x3F));
}
void chr_swap_mmc3_513(WORD address, WORD value) {
	chr_swap_MMC3_base(address, (value & 0x3F));
}
