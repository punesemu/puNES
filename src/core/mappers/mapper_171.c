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

INLINE static void prg_fix_171(void);
INLINE static void chr_fix_171(void);

struct _m171 {
	BYTE reg[2];
} m171;

void map_init_171(void) {
	EXTCL_AFTER_MAPPER_INIT(171);
	EXTCL_CPU_WR_MEM(171);
	EXTCL_SAVE_MAPPER(171);

	if (info.reset >= HARD) {
		memset(&m171, 0x00, sizeof(m171));
	}
}
void extcl_after_mapper_init_171(void) {
	prg_fix_171();
	chr_fix_171();
}
void extcl_cpu_wr_mem_171(UNUSED(BYTE nidx), WORD address, BYTE value) {
	m171.reg[address & 0x01] = value;
	chr_fix_171();
}
BYTE extcl_save_mapper_171(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m171.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_171(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_171(void) {
	memmap_auto_4k(0, MMPPU(0x0000), m171.reg[0]);
	memmap_auto_4k(0, MMPPU(0x1000), m171.reg[1]);
}
