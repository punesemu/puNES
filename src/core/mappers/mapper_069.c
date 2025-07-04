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

void prg_swap_fme7_069(WORD address, WORD value);
void chr_swap_fme7_069(WORD address, WORD value);

void map_init_069(void) {
	EXTCL_AFTER_MAPPER_INIT(FME7);
	EXTCL_CPU_WR_MEM(FME7);
	EXTCL_SAVE_MAPPER(FME7);
	EXTCL_CPU_EVERY_CYCLE(FME7);
	EXTCL_APU_TICK(FME7);
	map_internal_struct_init((BYTE *)&fme7, sizeof(fme7));

	init_FME7(info.reset);
	FME7_prg_swap = prg_swap_fme7_069;
	FME7_chr_swap = chr_swap_fme7_069;
}

void prg_swap_fme7_069(WORD address, WORD value) {
	prg_swap_FME7_base(address, (value & 0x3F));
}
void chr_swap_fme7_069(WORD address, WORD value) {
	chr_swap_FME7_base(address, (value & 0xFF));
}
