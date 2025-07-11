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

INLINE static void prg_fix_143(void);

void map_init_143(void) {
	EXTCL_AFTER_MAPPER_INIT(143);
	EXTCL_CPU_WR_MEM(143);
	EXTCL_CPU_RD_MEM(143);
}
void extcl_after_mapper_init_143(void) {
	prg_fix_143();
}
void extcl_cpu_wr_mem_143(UNUSED(BYTE nidx), UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_143(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x4100) && (address <= 0x5FFF)) {
		if (address & 0x0100) {
			return ((~address & 0x003F) | (openbus & 0xC0));
		} else if (address >= 0x5000) {
			return (0xFF);
		}
	}
	return (wram_rd(nidx, address));
}

INLINE static void prg_fix_143(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
