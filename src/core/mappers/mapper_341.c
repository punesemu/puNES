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

INLINE static void prg_fix_341(void);
INLINE static void chr_fix_341(void);
INLINE static void mirroring_fix_341(void);

struct _m341 {
	WORD reg;
} m341;

void map_init_341(void) {
	EXTCL_AFTER_MAPPER_INIT(341);
	EXTCL_CPU_WR_MEM(341);
	EXTCL_SAVE_MAPPER(341);
	map_internal_struct_init((BYTE *)&m341, sizeof(m341));

	memset(&m341, 0x00, sizeof(m341));
}
void extcl_after_mapper_init_341(void) {
	prg_fix_341();
	chr_fix_341();
	mirroring_fix_341();
}
void extcl_cpu_wr_mem_341(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m341.reg = address;
	prg_fix_341();
	chr_fix_341();
	mirroring_fix_341();
}
BYTE extcl_save_mapper_341(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m341.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_341(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m341.reg >> 8));
}
INLINE static void chr_fix_341(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m341.reg >> 8));
}
INLINE static void mirroring_fix_341(void) {
	if (m341.reg & (prgrom_size() & S256K ? 0x800 : 0x200)) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
