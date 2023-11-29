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

INLINE static void prg_fix_229(void);
INLINE static void chr_fix_229(void);
INLINE static void mirroring_fix_229(void);

struct _m229 {
	WORD reg;
} m229;

void map_init_229(void) {
	EXTCL_AFTER_MAPPER_INIT(229);
	EXTCL_CPU_WR_MEM(229);
	EXTCL_SAVE_MAPPER(229);
	map_internal_struct_init((BYTE *)&m229, sizeof(m229));

	if (info.reset >= HARD) {
		memset(&m229, 0x00, sizeof(m229));
	}
}
void extcl_after_mapper_init_229(void) {
	prg_fix_229();
	chr_fix_229();
	mirroring_fix_229();
}
void extcl_cpu_wr_mem_229(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m229.reg = address;
	prg_fix_229();
	chr_fix_229();
	mirroring_fix_229();
}
BYTE extcl_save_mapper_229(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m229.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_229(void) {
	if (m229.reg & 0x1E) {
		WORD bank = (m229.reg & 0x1F);

		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), 0);
	}
}
INLINE static void chr_fix_229(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m229.reg & 0x1F));
}
INLINE static void mirroring_fix_229(void) {
	if (m229.reg & 0x20) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
