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

INLINE static void prg_fix_360(void);
INLINE static void chr_fix_360(void);
INLINE static void mirroring_fix_360(void);

INLINE static void tmp_fix_360(BYTE max, BYTE index, const WORD *ds);

struct _m360tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m360tmp;

void map_init_360(void) {
	EXTCL_AFTER_MAPPER_INIT(360);
	EXTCL_CPU_WR_MEM(360);

	if (info.reset == RESET) {
		if (m360tmp.ds_used) {
			m360tmp.index = (m360tmp.index + 1) % m360tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m360tmp, 0x00, sizeof(m360tmp));

		{
			static WORD ds[] = {
				0,  2,  3,  4,
				5,  6,  7,  8,
				9, 10, 11, 12,
				13, 14, 15, 16,
				17, 18, 19, 20,
				21, 22, 23, 24,
				25, 26, 27, 28,
				29, 30, 31
			};

			tmp_fix_360(LENGTH(ds), 0, &ds[0]);
		}
	}
}
void extcl_after_mapper_init_360(void) {
	prg_fix_360();
	chr_fix_360();
	mirroring_fix_360();
}
void extcl_cpu_wr_mem_360(UNUSED(WORD address), UNUSED(BYTE value)) {}

INLINE static void prg_fix_360(void) {
	if (m360tmp.dipswitch[m360tmp.index] < 2) {
		memmap_auto_32k(MMCPU(0x8000), (m360tmp.dipswitch[m360tmp.index] >> 1));
	} else {
		memmap_auto_16k(MMCPU(0x8000), m360tmp.dipswitch[m360tmp.index]);
		memmap_auto_16k(MMCPU(0xC000), m360tmp.dipswitch[m360tmp.index]);
	}
}
INLINE static void chr_fix_360(void) {
	memmap_auto_8k(MMPPU(0x0000), m360tmp.dipswitch[m360tmp.index]);
}
INLINE static void mirroring_fix_360(void) {
	if (m360tmp.dipswitch[m360tmp.index] & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_360(BYTE max, BYTE index, const WORD *ds) {
	m360tmp.ds_used = TRUE;
	m360tmp.max = max;
	m360tmp.index = index;
	m360tmp.dipswitch = ds;
}
