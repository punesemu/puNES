/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include "mappers.h"
#include "mem_map.h"

void map_init_0(void) {
	EXTCL_CPU_WR_MEM(0);
	EXTCL_CPU_RD_MEM(0);
}
void extcl_cpu_wr_mem_0(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_0(WORD address, BYTE openbus, BYTE before) {
	if (address < 0x6000) {
		return (before);
	}
	return (openbus);
}
