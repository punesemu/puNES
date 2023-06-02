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

INLINE static void prg_fix_265(void);
INLINE static void mirroring_fix_265(void);

struct _m265 {
	WORD reg[2];
} m265;

void map_init_265(void) {
	EXTCL_AFTER_MAPPER_INIT(265);
	EXTCL_CPU_WR_MEM(265);
	EXTCL_SAVE_MAPPER(265);
	mapper.internal_struct[0] = (BYTE *)&m265;
	mapper.internal_struct_size[0] = sizeof(m265);

	memset(&m265, 0x00, sizeof(m265));
}
void extcl_after_mapper_init_265(void) {
	prg_fix_265();
	mirroring_fix_265();
}
void extcl_cpu_wr_mem_265(WORD address, BYTE value) {
	if (!(m265.reg[0] & 0x2000)) {
		m265.reg[0] = address;
		mirroring_fix_265();
	}
	m265.reg[1] = value;
	prg_fix_265();
}
BYTE extcl_save_mapper_265(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m265.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_265(void) {
	WORD bank = ((m265.reg[0] & 0x0060) >> 2) | ((m265.reg[0] & 0x0300) >> 3) | (m265.reg[1] & 0x07);

	if (m265.reg[0] & 0x0080) {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	} else {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), (bank | 0x07));
	}
}
INLINE static void mirroring_fix_265(void) {
	if (m265.reg[0] & 0x02) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
