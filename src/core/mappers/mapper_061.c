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

INLINE static void prg_fix_061(void);
INLINE static void mirroring_fix_061(void);

struct _m061 {
	WORD reg;
} m061;

void map_init_061(void) {
	EXTCL_AFTER_MAPPER_INIT(061);
	EXTCL_CPU_WR_MEM(061);
	EXTCL_SAVE_MAPPER(061);
	mapper.internal_struct[0] = (BYTE *)&m061;
	mapper.internal_struct_size[0] = sizeof(m061);

	memset(&m061, 0x00, sizeof(m061));
}
void extcl_after_mapper_init_061(void) {
	prg_fix_061();
	mirroring_fix_061();
}
void extcl_cpu_wr_mem_061(WORD address, UNUSED(BYTE value)) {
	m061.reg = address;
	prg_fix_061();
	mirroring_fix_061();
}
BYTE extcl_save_mapper_061(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m061.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_061(void) {
	WORD bank = ((m061.reg & 0x0F) << 1) | ((m061.reg & 0x20) >> 5);

	if (m061.reg & 0x10) {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	}
}
INLINE static void mirroring_fix_061(void) {
	if (m061.reg & 0x80) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
