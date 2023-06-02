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

INLINE static void prg_fix_349(void);
INLINE static void mirroring_fix_349(void);

struct _m349 {
	WORD reg;
} m349;

void map_init_349(void) {
	EXTCL_AFTER_MAPPER_INIT(349);
	EXTCL_CPU_WR_MEM(349);
	EXTCL_SAVE_MAPPER(349);
	mapper.internal_struct[0] = (BYTE *)&m349;
	mapper.internal_struct_size[0] = sizeof(m349);

	if (info.reset >= HARD) {
		memset(&m349, 0x00, sizeof(m349));
	}
}
void extcl_after_mapper_init_349(void) {
	prg_fix_349();
	mirroring_fix_349();
}
void extcl_cpu_wr_mem_349(WORD address, UNUSED(BYTE value)) {
	m349.reg = address;
	prg_fix_349();
	mirroring_fix_349();
}
BYTE extcl_save_mapper_349(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m349.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_349(void) {
	WORD bank = m349.reg & 0x1F;

	if (m349.reg & 0x0800) {
		memmap_auto_16k(MMCPU(0x8000), (bank | (m349.reg & (m349.reg & 0x40) >> 6)));
		memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
	} else if (m349.reg & 0x40) {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	}
}
INLINE static void mirroring_fix_349(void) {
	if (m349.reg & 0x80) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
