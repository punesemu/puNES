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

INLINE static void prg_fix_230(void);
INLINE static void mirroring_fix_230(void);

struct _m230 {
	BYTE reg;
	BYTE mode;
} m230;

void map_init_230(void) {
	EXTCL_AFTER_MAPPER_INIT(230);
	EXTCL_CPU_WR_MEM(230);
	EXTCL_SAVE_MAPPER(230);
	map_internal_struct_init((BYTE *)&m230, sizeof(m230));

	if (info.reset >= HARD) {
		memset(&m230, 0x00, sizeof(m230));
	} else {
		m230.reg = 0;
		m230.mode ^= 1;
	};
}
void extcl_after_mapper_init_230(void) {
	prg_fix_230();
	mirroring_fix_230();
}
void extcl_cpu_wr_mem_230(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m230.reg = value;
	prg_fix_230();
	mirroring_fix_230();
}
BYTE extcl_save_mapper_230(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m230.reg);
	save_slot_ele(mode, slot, m230.mode);
	return (EXIT_OK);
}

INLINE static void prg_fix_230(void) {
	if (!m230.mode) {
		memmap_auto_16k(0, MMCPU(0x8000), (m230.reg & 0x07));
		memmap_auto_16k(0, MMCPU(0xC000), 0x07);
	} else {
		WORD bank = (m230.reg & 0x1F) + 0x08;

		if (m230.reg & 0x20) {
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), bank);
		} else {
			memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
		}
	}
}
INLINE static void mirroring_fix_230(void) {
	if (!m230.mode) {
		mirroring_V(0);
	} else {
		if (m230.reg & 0x40) {
			mirroring_V(0);
		} else {
			mirroring_H(0);
		}
	}
}
