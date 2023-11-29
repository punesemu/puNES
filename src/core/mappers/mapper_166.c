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

INLINE static void prg_fix_166(void);

struct _m166 {
	BYTE reg[4];
} m166;

void map_init_166(void) {
	EXTCL_AFTER_MAPPER_INIT(166);
	EXTCL_CPU_WR_MEM(166);
	EXTCL_SAVE_MAPPER(166);
	map_internal_struct_init((BYTE *)&m166, sizeof(m166));

	if (info.reset >= HARD) {
		memset(&m166, 0x00, sizeof(m166));
	}
}
void extcl_after_mapper_init_166(void) {
	prg_fix_166();
}
void extcl_cpu_wr_mem_166(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m166.reg[(address & 0x6000) >> 13] = value;
	prg_fix_166();
}
BYTE extcl_save_mapper_166(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m166.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_166(void) {
	WORD bank = (((m166.reg[0] ^ m166.reg[1]) & 0x10) << 1) + ((m166.reg[2] ^ m166.reg[3]) & 0x1F);

	if (m166.reg[1] & 0x08) {
		memmap_auto_16k(0, MMCPU(0x8000), (bank & 0xFE));
		memmap_auto_16k(0, MMCPU(0xC000), (bank | 0x01));
	} if (m166.reg[1] & 0x04) {
		memmap_auto_16k(0, MMCPU(0x8000), 0x1F);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), 0x07);
	}
}
