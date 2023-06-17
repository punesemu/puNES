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

void prg_swap_ks202_142(WORD address, WORD value);

void map_init_142(void) {
	EXTCL_AFTER_MAPPER_INIT(KS202);
	EXTCL_CPU_WR_MEM(KS202);
	EXTCL_SAVE_MAPPER(KS202);
	EXTCL_CPU_EVERY_CYCLE(KS202);
	mapper.internal_struct[0] = (BYTE *)&ks202;
	mapper.internal_struct_size[0] = sizeof(ks202);

	init_KS202(info.reset);
	KS202_prg_swap = prg_swap_ks202_142;
}

void prg_swap_ks202_142(WORD address, WORD value) {
	prg_swap_KS202_base(address, (value & 0x0F));
}
