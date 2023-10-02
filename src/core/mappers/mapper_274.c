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

INLINE static void prg_fix_274(void);
INLINE static void mirroring_fix_274(void);

struct _m274 {
	BYTE reg[2];
	BYTE extra;
} m274;

void map_init_274(void) {
	EXTCL_AFTER_MAPPER_INIT(274);
	EXTCL_CPU_WR_MEM(274);
	EXTCL_SAVE_MAPPER(274);
	mapper.internal_struct[0] = (BYTE *)&m274;
	mapper.internal_struct_size[0] = sizeof(m274);

	memset(&m274, 0x00, sizeof(m274));

	m274.extra = 0x80;
}
void extcl_after_mapper_init_274(void) {
	prg_fix_274();
	mirroring_fix_274();
}
void extcl_cpu_wr_mem_274(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			m274.reg[0] = value;
			prg_fix_274();
			mirroring_fix_274();
			return;
		case 0xA000:
		case 0xC000:
		case 0xE000:
			m274.reg[1] = value;
			m274.extra = !(address & 0x4000) ? 0x80 : 0x00;
			prg_fix_274();
			return;
	}
}
BYTE extcl_save_mapper_274(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m274.reg);
	save_slot_ele(mode, slot, m274.extra);
	return (EXIT_OK);
}

INLINE static void prg_fix_274(void) {
	WORD bank = m274.extra
		? m274.extra | (m274.reg[0] & (prgrom_banks(S16K) - 1) & 0x0F)
		: (m274.reg[1] & 0x70) | (m274.reg[0] & 0x0F);

	memmap_auto_16k(0, MMCPU(0x8000), bank);
	memmap_auto_16k(0, MMCPU(0xC000), (m274.reg[1] & 0x7F));
}
INLINE static void mirroring_fix_274(void) {
	if (m274.reg[0] & 0x10) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
