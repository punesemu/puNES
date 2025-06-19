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

INLINE static void prg_fix_144(void);
INLINE static void chr_fix_144(void);

struct _m144 {
	BYTE reg;
} m144;

void map_init_144(void) {
	EXTCL_AFTER_MAPPER_INIT(144);
	EXTCL_CPU_WR_MEM(144);
	EXTCL_SAVE_MAPPER(144);
	map_internal_struct_init((BYTE *)&m144, sizeof(m144));

	if (info.reset >= HARD) {
		memset(&m144, 0x00, sizeof(m144));
	}
}
void extcl_after_mapper_init_144(void) {
	prg_fix_144();
	chr_fix_144();
}
void extcl_cpu_wr_mem_144(BYTE nidx, WORD address, BYTE value) {
	BYTE rd = prgrom_rd(nidx, address);

	// bus conflict
	m144.reg = rd & ((value & rd) | 0x01);
	prg_fix_144();
	chr_fix_144();
}
BYTE extcl_save_mapper_144(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m144.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_144(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m144.reg & 0x03));
}
INLINE static void chr_fix_144(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m144.reg & 0xF0) >> 4));
}
