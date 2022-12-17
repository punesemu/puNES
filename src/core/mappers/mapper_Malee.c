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
#include "mem_map.h"

struct maleetmp {
	BYTE *prg_6000;
	WORD mask;
} maleetmp;

void map_init_malee(void) {
	EXTCL_CPU_RD_MEM(malee);

	map_prg_rom_8k(4, 0, 0);
	maleetmp.prg_6000 = prg_size() > 0x8000 ? prg_rom() + 0x8000 : NULL;
	if (maleetmp.prg_6000) {
		maleetmp.mask = ((prg_size() - 0x8000) >= 0x2000) ? 0x1FFF : (prg_size() - 0x8000) - 1;
	}
	mirroring_V();
}
BYTE extcl_cpu_rd_mem_malee(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x67FF)) {
		return (maleetmp.prg_6000 ? maleetmp.prg_6000[address & maleetmp.mask] : openbus);
	}
	return (openbus);
}
