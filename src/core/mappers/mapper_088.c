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

void prg_swap_n118_088(WORD address, WORD value);
void chr_fix_n118_088(void);

void map_init_088(void) {
	EXTCL_AFTER_MAPPER_INIT(N118);
	EXTCL_CPU_WR_MEM(N118);
	EXTCL_SAVE_MAPPER(N118);
	mapper.internal_struct[0] = (BYTE *)&n118;
	mapper.internal_struct_size[0] = sizeof(n118);

	init_N118();
	N118_prg_swap = prg_swap_n118_088;
	N118_chr_fix = chr_fix_n118_088;
}

void prg_swap_n118_088(WORD address, WORD value) {
	prg_swap_N118_base(address, (value & 0x0F));
}
void chr_fix_n118_088(void) {
	map_chr_rom_2k(0x0000, ((n118.reg[0] & 0x3E) >> 1));
	map_chr_rom_2k(0x0800, ((n118.reg[1] & 0x3E) >> 1));
	map_chr_rom_1k(0x1000, (0x40 | n118.reg[2]));
	map_chr_rom_1k(0x1400, (0x40 | n118.reg[3]));
	map_chr_rom_1k(0x1800, (0x40 | n118.reg[4]));
	map_chr_rom_1k(0x1C00, (0x40 | n118.reg[5]));
}
