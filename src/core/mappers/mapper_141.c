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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_141(void);
INLINE static void chr_fix_141(void);
INLINE static void mirroring_fix_141(void);

struct _m141 {
	BYTE index;
	BYTE reg[8];
} m141;

void map_init_141(void) {
	EXTCL_AFTER_MAPPER_INIT(141);
	EXTCL_CPU_WR_MEM(141);
	EXTCL_SAVE_MAPPER(141);
	map_internal_struct_init((BYTE *)&m141, sizeof(m141));

	if (info.reset >= HARD) {
		memset(&m141, 0x00, sizeof(m141));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_141(void) {
	prg_fix_141();
	chr_fix_141();
	mirroring_fix_141();
}
void extcl_cpu_wr_mem_141(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address & 0x4000) && (address & 0x0100)) {
		if (address & 0x01) {
			m141.reg[m141.index & 0x07] = value;
			prg_fix_141();
			chr_fix_141();
			mirroring_fix_141();
		} else {
			m141.index = value;
		}
	}
}
BYTE extcl_save_mapper_141(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m141.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_141(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m141.reg[5]);
}
INLINE static void chr_fix_141(void) {
	if (chrrom_size()) {
		WORD base = m141.reg[4] << 3;

		memmap_auto_2k(0, MMPPU(0x0000), (((base | (m141.reg[(m141.reg[7] & 0x01) ? 0 : 0] & 0x07)) << 1) | 0));
		memmap_auto_2k(0, MMPPU(0x0800), (((base | (m141.reg[(m141.reg[7] & 0x01) ? 0 : 1] & 0x07)) << 1) | 1));
		memmap_auto_2k(0, MMPPU(0x1000), (((base | (m141.reg[(m141.reg[7] & 0x01) ? 0 : 2] & 0x07)) << 1) | 0));
		memmap_auto_2k(0, MMPPU(0x1800), (((base | (m141.reg[(m141.reg[7] & 0x01) ? 0 : 3] & 0x07)) << 1) | 1));
	} else {
		memmap_auto_8k(0, MMPPU(0x0000), 0);
	}
}
INLINE static void mirroring_fix_141(void) {
	switch (m141.reg[7] & 0x07) {
		case 0:
			mirroring_SCR0x3_SCR1x1(0);
			break;
		case 2:
			mirroring_H(0);
			break;
		default:
			mirroring_V(0);
			break;
		case 6:
			mirroring_SCR0(0);
			break;
	}
}
