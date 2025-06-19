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
#include "ines.h"

INLINE static void prg_fix_218(void);
INLINE static void chr_fix_218(void);
INLINE static void mirroring_fix_218(void);

INLINE BYTE va10(void);

void map_init_218(void) {
	EXTCL_AFTER_MAPPER_INIT(218);
}
void extcl_after_mapper_init_218(void) {
	prg_fix_218();
	chr_fix_218();
	mirroring_fix_218();
}

INLINE static void prg_fix_218(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_218(void) {
	for (int i = 0; i < 8; i++) {
		memmap_chrrom_nmt_1k(0, MMPPU(0x0000 | (i * S1K)), ((i >> va10()) & 0x01));
	}
}
INLINE static void mirroring_fix_218(void) {
	for (int i = 0; i < 8; i++) {
		memmap_nmt_1k(0, MMPPU(0x2000 | (i * S1K)), (((i | 0x08) >> va10()) & 0x01));
	}
}

INLINE BYTE va10(void) {
	switch (ines.flags[FL6] & 0x09) {
		case 0:
			return (1);
		default:
		case 1:
			return (0);
		case 8:
			return (2);
		case 9:
			return (3);
	}
}