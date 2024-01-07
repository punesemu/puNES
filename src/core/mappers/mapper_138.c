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

INLINE static void prg_fix_138(void);
INLINE static void chr_fix_138(void);
INLINE static void mirroring_fix_138(void);

struct _m138 {
	BYTE index;
	BYTE reg[8];
} m138;

void map_init_138(void) {
	EXTCL_AFTER_MAPPER_INIT(138);
	EXTCL_CPU_WR_MEM(138);
	EXTCL_SAVE_MAPPER(138);
	map_internal_struct_init((BYTE *)&m138, sizeof(m138));

	if (info.reset >= HARD) {
		memset(&m138, 0x00, sizeof(m138));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_138(void) {
	prg_fix_138();
	chr_fix_138();
	mirroring_fix_138();
}
void extcl_cpu_wr_mem_138(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address & 0x4000) && (address & 0x0100)) {
		if (address & 0x01) {
			m138.reg[m138.index & 0x07] = value;
			prg_fix_138();
			chr_fix_138();
			mirroring_fix_138();
		} else {
			m138.index = value;
		}
	}
}
BYTE extcl_save_mapper_138(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m138.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_138(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m138.reg[5]);
}
INLINE static void chr_fix_138(void) {
	WORD base = m138.reg[4] << 3;

	memmap_auto_2k(0, MMPPU(0x0000), (base | (m138.reg[(m138.reg[7] & 0x01) ? 0 : 0] & 0x07)));
	memmap_auto_2k(0, MMPPU(0x0800), (base | (m138.reg[(m138.reg[7] & 0x01) ? 0 : 1] & 0x07)));
	memmap_auto_2k(0, MMPPU(0x1000), (base | (m138.reg[(m138.reg[7] & 0x01) ? 0 : 2] & 0x07)));
	memmap_auto_2k(0, MMPPU(0x1800), (base | (m138.reg[(m138.reg[7] & 0x01) ? 0 : 3] & 0x07)));
}
INLINE static void mirroring_fix_138(void) {
	switch (m138.reg[7] & 0x07) {
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
