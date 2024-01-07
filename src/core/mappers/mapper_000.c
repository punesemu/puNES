/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

INLINE static void prg_fix_000(void);
INLINE static void wram_fix_000(void);

struct _m000tmp {
	BYTE nrom368;
} m000tmp;

void map_init_000(void) {
	EXTCL_AFTER_MAPPER_INIT(000);
	EXTCL_CPU_WR_MEM(000);

	// Alter Ego (World) (Aftermarket) (Homebrew) (Alt).nes
	m000tmp.nrom368 = (prgrom_banks(S16K) == 3);
}
void extcl_after_mapper_init_000(void) {
	prg_fix_000();
	wram_fix_000();
}
void extcl_cpu_wr_mem_000(UNUSED(BYTE nidx), UNUSED(WORD address), UNUSED(BYTE value)) {}

INLINE static void prg_fix_000(void) {
	if (m000tmp.nrom368) {
		memmap_prgrom_16k(0, MMCPU(0x8000), 1);
		memmap_prgrom_16k(0, MMCPU(0xC000), 2);
	} else {
		memmap_prgrom_32k(0, MMCPU(0x8000), 0);
	}
}
INLINE static void wram_fix_000(void) {
	if (m000tmp.nrom368) {
		memmap_prgrom_16k(0, MMCPU(0x4000), 0);
	} else {
		memmap_disable_8k(0, MMCPU(0x4000));
	}
}
