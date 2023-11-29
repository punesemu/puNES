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

INLINE static void prg_fix_058(void);
INLINE static void chr_fix_058(void);
INLINE static void mirroring_fix_058(void);

struct _m058 {
	WORD reg;
} m058;

void map_init_058(void) {
	EXTCL_AFTER_MAPPER_INIT(058);
	EXTCL_CPU_WR_MEM(058);
	EXTCL_SAVE_MAPPER(058);
	map_internal_struct_init((BYTE *)&m058, sizeof(m058));

	memset(&m058, 0x00, sizeof(m058));
}
void extcl_after_mapper_init_058(void) {
	prg_fix_058();
	chr_fix_058();
	mirroring_fix_058();
}
void extcl_cpu_wr_mem_058(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m058.reg = address;
	prg_fix_058();
	chr_fix_058();
	mirroring_fix_058();
}
BYTE extcl_save_mapper_058(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m058.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_058(void) {
	WORD bank = m058.reg & 0x07;

	memmap_auto_16k(0, MMCPU(0x8000), (bank & ~((~m058.reg & 0x40) >> 6)));
	memmap_auto_16k(0, MMCPU(0xC000), (bank |  ((~m058.reg & 0x40) >> 6)));
}
INLINE static void chr_fix_058(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m058.reg & 0x38) >> 3));
}
INLINE static void mirroring_fix_058(void) {
	if (m058.reg & 0x80) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
