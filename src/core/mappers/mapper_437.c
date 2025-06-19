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

INLINE static void prg_fix_437(void);
INLINE static void mirroring_fix_437(void);

struct _m437 {
	WORD reg[2];
} m437;

void map_init_437(void) {
	EXTCL_AFTER_MAPPER_INIT(437);
	EXTCL_CPU_WR_MEM(437);
	EXTCL_SAVE_MAPPER(437);
	map_internal_struct_init((BYTE *)&m437, sizeof(m437));

	if (info.reset >= HARD) {
		memset(&m437, 0x00, sizeof(m437));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_437(void) {
	prg_fix_437();
	mirroring_fix_437();
}
void extcl_cpu_wr_mem_437(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m437.reg[0] = address;
		prg_fix_437();
		mirroring_fix_437();
	} else if (address >= 0x8000) {
		// bus conflict
		m437.reg[1] = value & prgrom_rd(nidx, address);
		prg_fix_437();
	}
}
BYTE extcl_save_mapper_437(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m437.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_437(void) {
	WORD base = (m437.reg[0] & 0x0F) << 3;

	memmap_auto_16k(0, MMCPU(0x8000), (base | (m437.reg[1] & 0x07)));
	memmap_auto_16k(0, MMCPU(0xC000), (base | 0x07));
}
INLINE static void mirroring_fix_437(void) {
	if (m437.reg[0] & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
