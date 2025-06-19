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

void prg_swap_n118_076(WORD address, WORD value);
void chr_fix_n118_076(void);

void map_init_076(void) {
	EXTCL_AFTER_MAPPER_INIT(N118);
	EXTCL_CPU_WR_MEM(N118);
	EXTCL_SAVE_MAPPER(N118);
	map_internal_struct_init((BYTE *)&n118, sizeof(n118));

	init_N118(info.reset);
	N118_prg_swap = prg_swap_n118_076;
	N118_chr_fix = chr_fix_n118_076;
}

void prg_swap_n118_076(WORD address, WORD value) {
	prg_swap_N118_base(address, (value & 0x1F));
}
void chr_fix_n118_076(void) {
	memmap_auto_2k(0, MMPPU(0x0000), n118.reg[2]);
	memmap_auto_2k(0, MMPPU(0x0800), n118.reg[3]);
	memmap_auto_2k(0, MMPPU(0x1000), n118.reg[4]);
	memmap_auto_2k(0, MMPPU(0x1800), n118.reg[5]);
}
