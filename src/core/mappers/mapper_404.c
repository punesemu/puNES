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
#include "mem_map.h"
#include "save_slot.h"

void prg_swap_mmc1_404(WORD address, WORD value);
void chr_swap_mmc1_404(WORD address, WORD value);

struct _m404 {
	BYTE reg;
} m404;

void map_init_404(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(404);
	EXTCL_SAVE_MAPPER(404);
	mapper.internal_struct[0] = (BYTE *)&m404;
	mapper.internal_struct_size[0] = sizeof(m404);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&m404, 0x00, sizeof(m404));

	init_MMC1(MMC1A);
	MMC1_prg_swap = prg_swap_mmc1_404;
	MMC1_chr_swap = chr_swap_mmc1_404;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_404(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m404.reg & 0x80)) {
			m404.reg = value;
			MMC1_prg_fix();
			MMC1_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC1(address, value);
	}
}
BYTE extcl_save_mapper_404(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m404.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc1_404(WORD address, WORD value) {
	WORD base = m404.reg << 3;
	WORD mask = 0x0F >> ((m404.reg & 0x40) >> 6);

	prg_swap_MMC1_base(address, (base | (value & mask)));
}
void chr_swap_mmc1_404(WORD address, WORD value) {
	chr_swap_MMC1_base(address, ((m404.reg << 5) | (value & 0x1F)));
}
