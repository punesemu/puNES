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

INLINE static void prg_fix_434(void);
INLINE static void mirroring_fix_434(void);

struct _m434 {
	BYTE reg[2];
} m434;

void map_init_434(void) {
	EXTCL_AFTER_MAPPER_INIT(434);
	EXTCL_CPU_WR_MEM(434);
	EXTCL_SAVE_MAPPER(434);
	map_internal_struct_init((BYTE *)&m434, sizeof(m434));

	if (info.reset >= HARD) {
		memset(&m434, 0x00, sizeof(m434));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_434(void) {
	prg_fix_434();
	mirroring_fix_434();
}
void extcl_cpu_wr_mem_434(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m434.reg[0] = value;
		prg_fix_434();
		mirroring_fix_434();
	} else if (address >= 0x8000) {
		// bus conflict
		m434.reg[1] = value & prgrom_rd(nidx, address);
		prg_fix_434();
	}
}
BYTE extcl_save_mapper_434(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m434.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_434(void) {
	WORD base = (m434.reg[0] & 0x07) << 3;

	memmap_auto_16k(0, MMCPU(0x8000), (base | (m434.reg[1] & 0x07)));
	memmap_auto_16k(0, MMCPU(0xC000), (base | 0x07));
}
INLINE static void mirroring_fix_434(void) {
	if (m434.reg[0] & 0x20) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
