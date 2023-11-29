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

INLINE static void prg_fix_167(void);

struct _m167 {
	BYTE reg[4];
} m167;

void map_init_167(void) {
	EXTCL_AFTER_MAPPER_INIT(167);
	EXTCL_CPU_WR_MEM(167);
	EXTCL_SAVE_MAPPER(167);
	map_internal_struct_init((BYTE *)&m167, sizeof(m167));

	if (info.reset >= HARD) {
		memset(&m167, 0x00, sizeof(m167));
	}
}
void extcl_after_mapper_init_167(void) {
	prg_fix_167();
}
void extcl_cpu_wr_mem_167(UNUSED(BYTE nidx), WORD address, BYTE value) {
	m167.reg[(address & 0x6000) >> 13] = value;
	prg_fix_167();
}
BYTE extcl_save_mapper_167(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m167.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_167(void) {
	WORD bank = (((m167.reg[0] ^ m167.reg[1]) & 0x10) << 1) + ((m167.reg[2] ^ m167.reg[3]) & 0x1F);

	if (m167.reg[1] & 0x08) {
		memmap_auto_16k(0, MMCPU(0x8000), (bank | 0x01));
		memmap_auto_16k(0, MMCPU(0xC000), (bank & 0xFE));
	} else if (m167.reg[1] & 0x04) {
		memmap_auto_16k(0, MMCPU(0x8000), 0x1F);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), 0x20);
	}
}
