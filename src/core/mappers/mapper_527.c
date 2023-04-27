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
#include "mem_map.h"

void prg_swap_vrc2and4_527(WORD address, WORD value);
void chr_swap_vrc2and4_527(WORD address, WORD value);
void mirroring_fix_vrc2and4_527(void);

void map_init_527(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(VRC2and4);
	EXTCL_CPU_RD_MEM(VRC2and4);
	EXTCL_SAVE_MAPPER(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[0] = sizeof(vrc2and4);

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_527;
	VRC2and4_chr_swap = chr_swap_vrc2and4_527;
	VRC2and4_mirroring_fix = mirroring_fix_vrc2and4_527;
}

void prg_swap_vrc2and4_527(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_527(WORD address, WORD value) {
	BYTE slot = address >> 10;

	if (slot < 2) {
		slot <<= 1;
		ntbl.bank_1k[slot] = ntbl.bank_1k[slot | 0x01] = &ntbl.data[((value & 0x80) << 4)];
	}
	chr_swap_VRC2and4_base(address, (value & 0xFFF));
}
void mirroring_fix_vrc2and4_527(void) {}