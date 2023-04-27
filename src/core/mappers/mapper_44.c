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

void prg_swap_mmc3_44(WORD address, WORD value);
void chr_swap_mmc3_44(WORD address, WORD value);

struct _m44 {
	BYTE reg;
} m44;

void map_init_44(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(44);
	EXTCL_SAVE_MAPPER(44);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m44;
	mapper.internal_struct_size[0] = sizeof(m44);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m44, 0x00, sizeof(m44));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_44;
	MMC3_chr_swap = chr_swap_mmc3_44;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_44(WORD address, BYTE value) {
	if ((address & 0xE001) == 0xA001) {
		m44.reg = value & 0x07;
		MMC3_prg_fix();
		MMC3_chr_fix();
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_44(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m44.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_44(WORD address, WORD value) {
	WORD base = m44.reg << 4;
	WORD mask = m44.reg >= 6 ? 0x1F : 0x0F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_44(WORD address, WORD value) {
	WORD base = m44.reg << 7;
	WORD mask = m44.reg >= 6 ? 0xFF : 0x7F;

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
