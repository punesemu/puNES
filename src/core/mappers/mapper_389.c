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

INLINE static void prg_fix_389(void);
INLINE static void chr_fix_389(void);
INLINE static void mirroring_fix_389(void);

struct _m389 {
	WORD reg[3];
} m389;

void map_init_389(void) {
	EXTCL_AFTER_MAPPER_INIT(389);
	EXTCL_CPU_WR_MEM(389);
	EXTCL_SAVE_MAPPER(389);
	mapper.internal_struct[0] = (BYTE *)&m389;
	mapper.internal_struct_size[0] = sizeof(m389);

	memset(&m389, 0x00, sizeof(m389));
}
void extcl_after_mapper_init_389(void) {
	prg_fix_389();
	chr_fix_389();
	mirroring_fix_389();
}
void extcl_cpu_wr_mem_389(WORD address, UNUSED(BYTE value)) {
	switch (address & 0xF000) {
		case 0x8000:
			m389.reg[0] = address & 0xFF;
			prg_fix_389();
			mirroring_fix_389();
			break;
		case 0x9000:
			m389.reg[1] = address & 0xFF;
			prg_fix_389();
			chr_fix_389();
			break;
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m389.reg[2] = address & 0x0F;
			prg_fix_389();
			chr_fix_389();
			break;
	}
}
BYTE extcl_save_mapper_389(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m389.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_389(void) {
	if (m389.reg[1] & 0x02) {
		memmap_auto_16k(MMCPU(0x8000), ((m389.reg[0] >> 2) | ((m389.reg[2] & 0x0C) >> 2)));
		memmap_auto_16k(MMCPU(0xC000), ((m389.reg[0] >> 2) | 0x03));
	} else {
		memmap_auto_32k(MMCPU(0x8000), (m389.reg[0] >> 3));
	}
}
INLINE static void chr_fix_389(void) {
	memmap_auto_8k(MMPPU(0x0000), (((m389.reg[1] & 0x38) >> 1) | (m389.reg[2] & 0x03)));
}
INLINE static void mirroring_fix_389(void) {
	if (m389.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
