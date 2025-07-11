/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

void prg_swap_vrc2and4_021(WORD address, WORD value);
void chr_swap_vrc2and4_021(WORD address, WORD value);

void map_init_021(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(VRC2and4);
	EXTCL_SAVE_MAPPER(VRC2and4);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	map_internal_struct_init((BYTE *)&vrc2and4, sizeof(vrc2and4));

	switch (info.mapper.submapper) {
		default:
			init_VRC2and4(VRC24_VRC4, 0x42, 0x84, TRUE, info.reset);
			break;
		case 1:
			init_VRC2and4(VRC24_VRC4, 0x02, 0x04, TRUE, info.reset);
			break;
		case 2:
			init_VRC2and4(VRC24_VRC4, 0x40, 0x80, TRUE, info.reset);
			break;
	}
	VRC2and4_prg_swap = prg_swap_vrc2and4_021;
	VRC2and4_chr_swap = chr_swap_vrc2and4_021;
}

void prg_swap_vrc2and4_021(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_021(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}