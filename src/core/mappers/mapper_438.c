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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_438(void);
INLINE static void chr_fix_438(void);
INLINE static void mirroring_fix_438(void);

struct _m438 {
	WORD reg[2];
} m438;

void map_init_438(void) {
	EXTCL_AFTER_MAPPER_INIT(438);
	EXTCL_CPU_WR_MEM(438);
	EXTCL_SAVE_MAPPER(438);
	mapper.internal_struct[0] = (BYTE *)&m438;
	mapper.internal_struct_size[0] = sizeof(m438);

	memset(&m438, 0x00, sizeof(m438));
}
void extcl_after_mapper_init_438(void) {
	prg_fix_438();
	chr_fix_438();
	mirroring_fix_438();
}
void extcl_cpu_wr_mem_438(WORD address, BYTE value) {
	m438.reg[0] = address;
	m438.reg[1] = value;
	prg_fix_438();
	chr_fix_438();
	mirroring_fix_438();
}
BYTE extcl_save_mapper_438(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m438.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_438(void) {
	WORD bank = m438.reg[0] >> 1;

	memmap_auto_16k(MMCPU(0x8000), (bank & ~(m438.reg[0] & 0x01)));
	memmap_auto_16k(MMCPU(0xC000), (bank |  (m438.reg[0] & 0x01)));
}
INLINE static void chr_fix_438(void) {
	memmap_auto_8k(MMPPU(0x0000), (m438.reg[1] >> 1));
}
INLINE static void mirroring_fix_438(void) {
	if (m438.reg[1] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
