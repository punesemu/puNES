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

INLINE static void prg_fix_283(void);
INLINE static void wram_fix_283(void);

struct _m283 {
	BYTE reg;
} m283;

void map_init_283(void) {
	EXTCL_AFTER_MAPPER_INIT(283);
	EXTCL_CPU_WR_MEM(283);

	memset(&m283, 0x00, sizeof(m283));
}
void extcl_after_mapper_init_283(void) {
	prg_fix_283();
	wram_fix_283();
}
void extcl_cpu_wr_mem_283(UNUSED(WORD address), BYTE value) {
	m283.reg = value;
	prg_fix_283();
}

INLINE static void prg_fix_283(void) {
	memmap_auto_32k(MMCPU(0x8000), m283.reg);
}
INLINE static void wram_fix_283(void) {
	memmap_prgrom_8k(MMCPU(0x6000), (prgrom_size() & 0x6000 ? 0x20 : 0x1F));
}
