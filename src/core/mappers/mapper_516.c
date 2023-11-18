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
#include "save_slot.h"

void prg_swap_mmc3_516(WORD address, WORD value);
void chr_swap_mmc3_516(WORD address, WORD value);

struct _m516 {
	BYTE reg;
} m516;

void map_init_516(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(516);
	EXTCL_SAVE_MAPPER(516);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m516;
	mapper.internal_struct_size[0] = sizeof(m516);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m516, 0x00, sizeof(m516));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_516;
	MMC3_chr_swap = chr_swap_mmc3_516;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_516(BYTE nidx, WORD address, BYTE value) {
	if (address & 0x0010) {
		m516.reg = address & 0x0F;
		MMC3_prg_fix();
		MMC3_chr_fix();
	}
	extcl_cpu_wr_mem_MMC3(nidx, address, value);
}
BYTE extcl_save_mapper_516(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m516.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_516(WORD address, WORD value) {
	WORD base = (m516.reg & 0x03) << 4;
	WORD mask = 0x0F;

	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_516(WORD address, WORD value) {
	WORD base = (m516.reg & 0x0C) << 5;
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
