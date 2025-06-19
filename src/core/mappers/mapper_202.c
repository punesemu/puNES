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

INLINE static void prg_fix_202(void);
INLINE static void chr_fix_202(void);
INLINE static void mirroring_fix_202(void);

struct _m202 {
	WORD reg;
} m202;

void map_init_202(void) {
	EXTCL_AFTER_MAPPER_INIT(202);
	EXTCL_CPU_WR_MEM(202);
	EXTCL_SAVE_MAPPER(202);

	if (info.reset >= HARD) {
		memset(&m202, 0x00, sizeof(m202));
	}
}
void extcl_after_mapper_init_202(void) {
	prg_fix_202();
	chr_fix_202();
	mirroring_fix_202();
}
void extcl_cpu_wr_mem_202(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m202.reg = address;
	prg_fix_202();
	chr_fix_202();
	mirroring_fix_202();
}
BYTE extcl_save_mapper_202(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m202.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_202(void) {
	WORD bank = m202.reg >> 1;

	if ((m202.reg & 0x09) == 0x09) {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_202(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m202.reg >> 1));
}
INLINE static void mirroring_fix_202(void) {
	if (m202.reg & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
