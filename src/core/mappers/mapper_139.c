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
#include "save_slot.h"

INLINE static void prg_fix_139(void);
INLINE static void chr_fix_139(void);
INLINE static void mirroring_fix_139(void);

struct _m139 {
	BYTE index;
	BYTE reg[8];
} m139;

void map_init_139(void) {
	EXTCL_AFTER_MAPPER_INIT(139);
	EXTCL_CPU_WR_MEM(139);
	EXTCL_SAVE_MAPPER(139);
	mapper.internal_struct[0] = (BYTE *)&m139;
	mapper.internal_struct_size[0] = sizeof(m139);

	if (info.reset >= HARD) {
		memset(&m139, 0x00, sizeof(m139));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_139(void) {
	prg_fix_139();
	chr_fix_139();
	mirroring_fix_139();
}
void extcl_cpu_wr_mem_139(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address & 0x4000) && (address & 0x0100)) {
		if (address & 0x01) {
			m139.reg[m139.index & 0x07] = value;
			prg_fix_139();
			chr_fix_139();
			mirroring_fix_139();
		} else {
			m139.index = value;
		}
	}
}
BYTE extcl_save_mapper_139(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m139.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_139(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m139.reg[5]);
}
INLINE static void chr_fix_139(void) {
	WORD base = m139.reg[4] << 3;

	memmap_auto_2k(0, MMPPU(0x0000), (((base | (m139.reg[(m139.reg[7] & 0x01) ? 0 : 0] & 0x07)) << 2) | 0));
	memmap_auto_2k(0, MMPPU(0x0800), (((base | (m139.reg[(m139.reg[7] & 0x01) ? 0 : 1] & 0x07)) << 2) | 1));
	memmap_auto_2k(0, MMPPU(0x1000), (((base | (m139.reg[(m139.reg[7] & 0x01) ? 0 : 2] & 0x07)) << 2) | 2));
	memmap_auto_2k(0, MMPPU(0x1800), (((base | (m139.reg[(m139.reg[7] & 0x01) ? 0 : 3] & 0x07)) << 2) | 3));
}
INLINE static void mirroring_fix_139(void) {
	switch (m139.reg[7] & 0x07) {
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
