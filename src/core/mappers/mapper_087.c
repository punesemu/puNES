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

INLINE static void prg_fix_087(void);
INLINE static void chr_fix_087(void);

struct _m087 {
	BYTE reg;
} m087;

void map_init_087(void) {
	EXTCL_AFTER_MAPPER_INIT(087);
	EXTCL_CPU_WR_MEM(087);
	EXTCL_SAVE_MAPPER(087);

	if (info.reset >= HARD) {
		memset(&m087, 0x00, sizeof(m087));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_087(void) {
	prg_fix_087();
	chr_fix_087();
}
void extcl_cpu_wr_mem_087(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m087.reg = value;
		chr_fix_087();
	}
}
BYTE extcl_save_mapper_087(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m087.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_087(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
INLINE static void chr_fix_087(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (((m087.reg & 0x01) << 1) | ((m087.reg & 0x02) >> 1)));
}
