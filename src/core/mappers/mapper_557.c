/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

INLINE static void mirroring_fix_557(void);

void map_init_557(void) {
	map_init_N118();

	EXTCL_AFTER_MAPPER_INIT(557);
}
void extcl_after_mapper_init_557(void) {
	prg_fix_N118(0x000F, 0x00);
	mirroring_fix_557();
}

INLINE static void mirroring_fix_557(void) {
	if (n118.reg[5] & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
