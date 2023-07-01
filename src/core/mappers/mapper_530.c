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

#include "mappers.h"
#include "info.h"

void prg_swap_vrc2and4_530(WORD address, WORD value);
void chr_swap_vrc2and4_530(WORD address, WORD value);

void map_init_530(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(530);
	EXTCL_SAVE_MAPPER(VRC2and4);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);

	init_VRC2and4(VRC24_VRC4, 0x01, 0x02, TRUE, info.reset);
	VRC2and4_prg_swap = prg_swap_vrc2and4_530;
	VRC2and4_chr_swap = chr_swap_vrc2and4_530;
}
void extcl_cpu_wr_mem_530(WORD address, BYTE value) {
	address |= ((address & 0x0008) << 9);

	switch (address & 0xF001) {
		case 0x8000:
		case 0x8001:
		case 0xA000:
		case 0xA001:
			extcl_cpu_wr_mem_VRC2and4(address, ((value & 0x02) << 2) | ((value & 0x08) >> 2) | (value & 0x05));
			return;
		case 0xB001:
		case 0xC001:
		case 0xD001:
		case 0xE001:
			extcl_cpu_wr_mem_VRC2and4(address, ((value & 0x04) >> 1) | ((value & 0x02) << 1) | (value & 0x09));
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
	}
}

void prg_swap_vrc2and4_530(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_530(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}
