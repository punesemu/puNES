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

INLINE static void prg_fix_481(void);

struct _m481 {
	BYTE reg;
} m481;

void map_init_481(void) {
	EXTCL_AFTER_MAPPER_INIT(481);
	EXTCL_CPU_WR_MEM(481);
	EXTCL_SAVE_MAPPER(481);

	if (info.reset >= HARD) {
		memset(&m481, 0x00, sizeof(m481));
	}
}
void extcl_after_mapper_init_481(void) {
	prg_fix_481();
}
void extcl_cpu_wr_mem_481(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m481.reg = value;
	prg_fix_481();
}
BYTE extcl_save_mapper_481(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m481.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_481(void) {
	WORD bank = ((m481.reg & 0x80) >> 4);

	memmap_auto_16k(0, MMCPU(0x8000), (bank | (m481.reg & 0x07)));
	memmap_auto_16k(0, MMCPU(0xC000), (bank | 0x07));
}
