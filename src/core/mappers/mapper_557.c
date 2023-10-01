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

void prg_swap_n118_557(WORD address, WORD value);
void chr_fix_n118_557(void);
void chr_swap_n118_557(WORD address, WORD value);

INLINE static void mirroring_fix_557(void);

void map_init_557(void) {
	EXTCL_AFTER_MAPPER_INIT(N118);
	EXTCL_CPU_WR_MEM(N118);
	EXTCL_SAVE_MAPPER(N118);
	mapper.internal_struct[0] = (BYTE *)&n118;
	mapper.internal_struct_size[0] = sizeof(n118);

	init_N118(info.reset);
	N118_prg_swap = prg_swap_n118_557;
	N118_chr_fix = chr_fix_n118_557;
	N118_chr_swap = chr_swap_n118_557;
}

void prg_swap_n118_557(WORD address, WORD value) {
	prg_swap_N118_base(address, (value & 0x0F));
}
void chr_fix_n118_557(void) {
	chr_fix_N118_base();
	mirroring_fix_557();
}
void chr_swap_n118_557(WORD address, WORD value) {
	value = (address >> 10) & 0x07;
	chr_swap_N118_base(address, value);
}

INLINE static void mirroring_fix_557(void) {
	if (n118.reg[5] & 0x20) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}