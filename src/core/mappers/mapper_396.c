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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_396(void);
INLINE static void mirroring_fix_396(void);

struct _m396 {
	BYTE reg[2];
} m396;

void map_init_396(void) {
	EXTCL_AFTER_MAPPER_INIT(396);
	EXTCL_CPU_WR_MEM(396);
	EXTCL_SAVE_MAPPER(396);
	map_internal_struct_init((BYTE *)&m396, sizeof(m396));

	memset(&m396, 0x00, sizeof(m396));
}
void extcl_after_mapper_init_396(void) {
	prg_fix_396();
	mirroring_fix_396();
}
void extcl_cpu_wr_mem_396(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0xA000) && (address <= 0xBFFF)) {
		m396.reg[1] = value;
	} else {
		m396.reg[0] = value;
	}
	prg_fix_396();
	mirroring_fix_396();
}
BYTE extcl_save_mapper_396(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m396.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_396(void) {
	WORD base = (m396.reg[1] & 0x0F) << 3;

	memmap_auto_16k(0, MMCPU(0x8000), (base | (m396.reg[0] & 0x07)));
	memmap_auto_16k(0, MMCPU(0xC000), (base | 0x07));
}
INLINE static void mirroring_fix_396(void) {
	if (m396.reg[1] & 0x60) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
