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

INLINE static void prg_fix_244(void);
INLINE static void chr_fix_244(void);

struct _m244 {
	BYTE reg[2];
} m244;

void map_init_244(void) {
	EXTCL_AFTER_MAPPER_INIT(244);
	EXTCL_CPU_WR_MEM(244);
	EXTCL_SAVE_MAPPER(244);
	mapper.internal_struct[0] = (BYTE *)&m244;
	mapper.internal_struct_size[0] = sizeof(m244);

	if (info.reset >= HARD) {
		memset(&m244, 0x00, sizeof(m244));
	}
}
void extcl_after_mapper_init_244(void) {
	prg_fix_244();
	chr_fix_244();
}
void extcl_cpu_wr_mem_244(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	if (value & 0x08) {
		static const BYTE chr_bank[8][8] ={
			{ 0, 1, 2, 3, 4, 5, 6, 7, },
			{ 0, 2, 1, 3, 4, 6, 5, 7, },
			{ 0, 1, 4, 5, 2, 3, 6, 7, },
			{ 0, 4, 1, 5, 2, 6, 3, 7, },
			{ 0, 4, 2, 6, 1, 5, 3, 7, },
			{ 0, 2, 4, 6, 1, 3, 5, 7, },
			{ 7, 6, 5, 4, 3, 2, 1, 0, },
			{ 7, 6, 5, 4, 3, 2, 1, 0, }
		};

		m244.reg[1] = chr_bank[((value & 0x70) >> 4)][value & 0x07];
		chr_fix_244();
		return;
	} else {
		static const BYTE prg_bank[4][4] ={
			{ 0, 1, 2, 3, },
			{ 3, 2, 1, 0, },
			{ 0, 2, 1, 3, },
			{ 3, 1, 2, 0, },
		};

		m244.reg[0] = prg_bank[((value & 0x30) >> 4)][value & 0x03];
		prg_fix_244();
		return;
	}
}
BYTE extcl_save_mapper_244(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m244.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_244(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m244.reg[0]);
}
INLINE static void chr_fix_244(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m244.reg[1]);
}
