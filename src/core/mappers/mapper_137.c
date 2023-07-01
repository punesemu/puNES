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

INLINE static void prg_fix_137(void);
INLINE static void chr_fix_137(void);
INLINE static void mirroring_fix_137(void);

struct _m137 {
	BYTE index;
	BYTE reg[8];
} m137;

void map_init_137(void) {
	EXTCL_AFTER_MAPPER_INIT(137);
	EXTCL_CPU_WR_MEM(137);
	EXTCL_SAVE_MAPPER(137);
	mapper.internal_struct[0] = (BYTE *)&m137;
	mapper.internal_struct_size[0] = sizeof(m137);

	if (info.reset >= HARD) {
		memset(&m137, 0x00, sizeof(m137));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_137(void) {
	prg_fix_137();
	chr_fix_137();
	mirroring_fix_137();
}
void extcl_cpu_wr_mem_137(WORD address, BYTE value) {
	if ((address & 0x4000) && (address & 0x0100)) {
		if (address & 0x01) {
			m137.reg[m137.index & 0x07] = value;
			prg_fix_137();
			chr_fix_137();
			mirroring_fix_137();
		} else {
			m137.index = value;
		}
	}
}
BYTE extcl_save_mapper_137(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m137.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_137(void) {
	memmap_auto_32k(MMCPU(0x8000), m137.reg[5]);
}
INLINE static void chr_fix_137(void) {
	memmap_auto_1k(MMPPU(0x0000), (m137.reg[(m137.reg[7] & 0x01) ? 0 : 0] & 0x07));
	memmap_auto_1k(MMPPU(0x0400), (((m137.reg[4] & 0x01) << 4) | (m137.reg[(m137.reg[7] & 0x01) ? 0 : 1] & 0x07)));
	memmap_auto_1k(MMPPU(0x0800), (((m137.reg[4] & 0x02) << 3) | (m137.reg[(m137.reg[7] & 0x01) ? 0 : 2] & 0x07)));
	memmap_auto_1k(MMPPU(0x0C00), (((m137.reg[4] & 0x04) << 2) | (((m137.reg[6] & 0x01) << 3)) | (m137.reg[(m137.reg[7] & 0x01) ? 0 : 3] & 0x07)));
	memmap_auto_4k(MMPPU(0x1000), 0xFF);
}
INLINE static void mirroring_fix_137(void) {
	switch (m137.reg[7] & 0x07) {
		default:
			mirroring_H();
			break;
		case 2:
			mirroring_V();
			break;
		case 4:
			mirroring_SCR0x3_SCR1x1();
			break;
		case 6:
			mirroring_SCR0();
			break;
	}
}
