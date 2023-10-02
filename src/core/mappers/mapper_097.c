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

INLINE static void prg_fix_097(void);
INLINE static void mirroring_fix_097(void);

struct m097 {
	BYTE reg;
} m097;

void map_init_097() {
	EXTCL_AFTER_MAPPER_INIT(097);
	EXTCL_CPU_WR_MEM(097);
	EXTCL_SAVE_MAPPER(097);
	mapper.internal_struct[0] = (BYTE *)&m097;
	mapper.internal_struct_size[0] = sizeof(m097);

	if (info.reset >= HARD) {
		memset(&m097, 0x00, sizeof(m097));
	}
}
void extcl_after_mapper_init_097(void) {
	prg_fix_097();
	mirroring_fix_097();
}
void extcl_cpu_wr_mem_097(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m097.reg = value;
	prg_fix_097();
	mirroring_fix_097();
}
BYTE extcl_save_mapper_097(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m097.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_097(void) {
	memmap_auto_16k(0, MMCPU(0x8000), 0xFF);
	memmap_auto_16k(0, MMCPU(0xC000), m097.reg);
}
INLINE static void mirroring_fix_097(void) {
	if (m097.reg & 0x80) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}

