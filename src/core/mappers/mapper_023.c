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

void prg_swap_vrc2and4_023(WORD address, WORD value);
void chr_swap_vrc2and4_023(WORD address, WORD value);
void wired_fix_vrc2and4_023(void);

void map_init_023(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(VRC2and4);
	EXTCL_SAVE_MAPPER(VRC2and4);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[0] = sizeof(vrc2and4);

	switch (info.mapper.submapper) {
		default:
			init_VRC2and4(VRC24_VRC4, 0x05, 0x0A, TRUE, info.reset);
			break;
		case 1:
			init_VRC2and4(VRC24_VRC4, 0x01, 0x02, TRUE, info.reset);
			break;
		case 2:
			init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE, info.reset);
			break;
		case 3:
			EXTCL_CPU_RD_MEM(023);
			init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE, info.reset);
			break;
	}
	VRC2and4_prg_swap = prg_swap_vrc2and4_023;
	VRC2and4_chr_swap = chr_swap_vrc2and4_023;
	VRC2and4_wired_fix = wired_fix_vrc2and4_023;
}
BYTE extcl_cpu_rd_mem_023(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	return (extcl_cpu_rd_mem_VRC2and4(nidx, address, wram_rd(nidx, address)));
}

void prg_swap_vrc2and4_023(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_023(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}
void wired_fix_vrc2and4_023(void) {
	vrc2and4.wired = ((vrc2and4.wired & 0x01) << 3) | (vrc2and4.wired & 0xF7);
}