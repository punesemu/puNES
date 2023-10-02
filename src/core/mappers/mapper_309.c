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

INLINE static void prg_fix_309(void);
INLINE static void mirroring_fix_309(void);

struct _m309 {
	BYTE reg[2];
} m309;

void map_init_309(void) {
	EXTCL_AFTER_MAPPER_INIT(309);
	EXTCL_CPU_WR_MEM(309);
	EXTCL_SAVE_MAPPER(309);
	mapper.internal_struct[0] = (BYTE *)&m309;
	mapper.internal_struct_size[0] = sizeof(m309);

	if (info.reset >= HARD) {
		memset(&m309, 0x00, sizeof(m309));
	}
}
void extcl_after_mapper_init_309(void) {
	prg_fix_309();
	mirroring_fix_309();
}
void extcl_cpu_wr_mem_309(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m309.reg[0] = value;
			prg_fix_309();
			return;
		case 0xF000:
			m309.reg[1] = value;
			mirroring_fix_309();
			return;
	}
}
BYTE extcl_save_mapper_309(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m309.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_309(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m309.reg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), 0xFD);
	memmap_auto_8k(0, MMCPU(0xC000), 0xFE);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void mirroring_fix_309(void) {
	if (m309.reg[1] & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
