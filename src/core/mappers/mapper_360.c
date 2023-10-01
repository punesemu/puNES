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

INLINE static void prg_fix_360(void);
INLINE static void chr_fix_360(void);
INLINE static void mirroring_fix_360(void);

void map_init_360(void) {
	EXTCL_AFTER_MAPPER_INIT(360);
	EXTCL_CPU_WR_MEM(360);
}
void extcl_after_mapper_init_360(void) {
	prg_fix_360();
	chr_fix_360();
	mirroring_fix_360();
}
void extcl_cpu_wr_mem_360(UNUSED(BYTE nidx), UNUSED(WORD address), UNUSED(BYTE value)) {}

INLINE static void prg_fix_360(void) {
	if (dipswitch.value < 2) {
		memmap_auto_32k(0, MMCPU(0x8000), (dipswitch.value >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), dipswitch.value);
		memmap_auto_16k(0, MMCPU(0xC000), dipswitch.value);
	}
}
INLINE static void chr_fix_360(void) {
	memmap_auto_8k(0, MMPPU(0x0000), dipswitch.value);
}
INLINE static void mirroring_fix_360(void) {
	if (dipswitch.value & 0x10) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
