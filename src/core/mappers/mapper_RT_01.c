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

#include <stdlib.h>
#include "mappers.h"
#include "info.h"

void map_init_RT_01(void) {
	EXTCL_CPU_RD_MEM(RT_01);

	info.mapper.extend_rd = TRUE;
}
BYTE extcl_cpu_rd_mem_RT_01(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (((address >= 0xCE80) && (address < 0xCF00)) || ((address >= 0xFE80) && (address < 0xFF00))) {
		return (0xF2 | (rand() & 0x0D));
	}
	return (openbus);
}
