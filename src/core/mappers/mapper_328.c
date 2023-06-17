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

#include <stdlib.h>
#include "mappers.h"
#include "info.h"

INLINE static void prg_fix_328(void);

void map_init_328(void) {
	EXTCL_AFTER_MAPPER_INIT(328);
	EXTCL_CPU_RD_MEM(328);

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_328(void) {
	prg_fix_328();
}
BYTE extcl_cpu_rd_mem_328(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		if (((address >= 0xCE80) && (address < 0xCF00)) || ((address >= 0xFE80) && (address < 0xFF00))) {
			return (0xF2 | (rand() & 0x0D));
		}
		return (prgrom_rd(address));
	}
	return (wram_rd(address));
}

INLINE static void prg_fix_328(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
