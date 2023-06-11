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

INLINE static void prg_fix_051(void);
INLINE static void wram_fix_051(void);
INLINE static void mirroring_fix_051(void);

struct _m051 {
	BYTE reg;
	BYTE mode;
} m051;

void map_init_051(void) {
	EXTCL_AFTER_MAPPER_INIT(051);
	EXTCL_CPU_WR_MEM(051);
	EXTCL_SAVE_MAPPER(051);
	mapper.internal_struct[0] = (BYTE *)&m051;
	mapper.internal_struct_size[0] = sizeof(m051);

	if (info.reset >= HARD) {
		memset(&m051, 0x00, sizeof(m051));
	}

	m051.mode = 2;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_051(void) {
	prg_fix_051();
	wram_fix_051();
	mirroring_fix_051();
}
void extcl_cpu_wr_mem_051(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m051.mode = value;
		prg_fix_051();
		wram_fix_051();
		mirroring_fix_051();
		return;
	}
	if (address >= 0x8000) {
		m051.reg = value;
		prg_fix_051();
		wram_fix_051();
	}
}
BYTE extcl_save_mapper_051(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m051.reg);
	save_slot_ele(mode, slot, m051.mode);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_051();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_051(void) {
	if (m051.mode & 0x02) {
		memmap_auto_32k(MMCPU(0x8000), (m051.reg >> ((info.mapper.submapper == 1) ? 1: 0)));
	} else {
		WORD base = (info.mapper.submapper == 1)
			? ((m051.reg & 0x10) >> 1) | ((m051.reg & 0x40) >> 2)
			: (m051.reg << 1);
		WORD prg = (info.mapper.submapper == 1)
			? (m051.reg & 0x07)
			: (m051.reg >> 4);

		memmap_auto_16k(MMCPU(0x8000), (base | prg));
		memmap_auto_16k(MMCPU(0xC000), (base | 0x07));
	}
}
INLINE static void wram_fix_051(void) {
	if (info.mapper.submapper == 1) {
		memmap_prgrom_8k(MMCPU(0x6000), ((m051.reg << 1) | 0x23));
	} else if (m051.mode & 0x02) {
		memmap_prgrom_8k(MMCPU(0x6000), (((m051.reg & 0x07) << 2) | 0x23));
	} else {
		memmap_prgrom_8k(MMCPU(0x6000), (((m051.reg & 0x04) << 2) | 0x2F));
	}
}
INLINE static void mirroring_fix_051(void) {
	if (m051.mode & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
