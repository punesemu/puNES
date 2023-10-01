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

INLINE static void prg_fix_335(void);
INLINE static void chr_fix_335(void);
INLINE static void mirroring_fix_335(void);

struct _m335 {
	BYTE reg[2];
} m335;

void map_init_335(void) {
	EXTCL_AFTER_MAPPER_INIT(335);
	EXTCL_CPU_WR_MEM(335);
	EXTCL_SAVE_MAPPER(335);
	mapper.internal_struct[0] = (BYTE *)&m335;
	mapper.internal_struct_size[0] = sizeof(m335);

	if (info.reset >= HARD) {
		memset(&m335, 0x00, sizeof(m335));
	}
}
void extcl_after_mapper_init_335(void) {
	prg_fix_335();
	chr_fix_335();
	mirroring_fix_335();
}
void extcl_cpu_wr_mem_335(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			m335.reg[0] = value;
			chr_fix_335();
			break;
		case 0xC000:
		case 0xE000:
			m335.reg[1] = value;
			prg_fix_335();
			mirroring_fix_335();
			break;
	}
}
BYTE extcl_save_mapper_335(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m335.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_335(void) {
	if (m335.reg[1] & 0x10) {
		WORD bank = ((m335.reg[1] & 0x07) << 1) | ((m335.reg[1] & 0x08) >> 3);

		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), (m335.reg[1] & 0x07));
	}
}
INLINE static void chr_fix_335(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m335.reg[0]);
}
INLINE static void mirroring_fix_335(void) {
	if (m335.reg[1] & 0x20) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
