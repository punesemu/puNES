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

void prg_swap_vrc7v2_085(WORD address, WORD value);
void chr_swap_vrc7v2_085(WORD address, WORD value);

void map_init_085(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC7);
	EXTCL_CPU_WR_MEM(VRC7);
	EXTCL_SAVE_MAPPER(VRC7);
	EXTCL_CPU_EVERY_CYCLE(VRC7);
	EXTCL_APU_TICK(VRC7);
	map_internal_struct_init((BYTE *)&vrc7, sizeof(vrc7));

	switch(info.mapper.submapper) {
		case 1:
			init_VRC7(0x08, 0x20, info.reset);
			break;
		case 2:
			init_VRC7(0x10, 0x20, info.reset);
			break;
		default:
			init_VRC7(0x18, 0x20, info.reset);
			break;
	}
	VRC7_prg_swap = prg_swap_vrc7v2_085;
	VRC7_chr_swap = chr_swap_vrc7v2_085;
}

void prg_swap_vrc7v2_085(WORD address, WORD value) {
	prg_swap_VRC7_base(address, (value & 0x3F));
}
void chr_swap_vrc7v2_085(WORD address, WORD value) {
	chr_swap_VRC7_base(address, (value & 0xFF));
}
