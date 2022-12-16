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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

void map_init_AX40G(void) {
	map_init_VRC2(VRC2B, 0x1F);

	EXTCL_CPU_WR_MEM(AX40G);
	EXTCL_SAVE_MAPPER(AX40G);
}
void extcl_cpu_wr_mem_AX40G(WORD address, BYTE value) {
	extcl_cpu_wr_mem_VRC2(address, value);

	if ((address & 0x7001) == 0x3001) {
		BYTE bank = (address & 0x02);

		ntbl.bank_1k[bank] = ntbl.bank_1k[bank | 1] = &ntbl.data[((value & 0x04) << 8)];
	}
}
BYTE extcl_save_mapper_AX40G(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_VRC2(mode, slot, fp);

	return (EXIT_OK);
}
