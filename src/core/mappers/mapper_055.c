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

INLINE static void prg_fix_055(void);
INLINE static void wram_fix_055(void);

void map_init_055(void) {
	EXTCL_AFTER_MAPPER_INIT(055);
}
void extcl_after_mapper_init_055(void) {
	prg_fix_055();
	wram_fix_055();
}

INLINE static void prg_fix_055(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
INLINE static void wram_fix_055(void) {
	memmap_prgrom_4k(MMCPU(0x6000), 8);
	memmap_auto_4k(MMCPU(0x7000), 0);
}
