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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_261(void);
INLINE static void chr_fix_261(void);

struct m261 {
	WORD reg;
} m261;

void map_init_261(void) {
	EXTCL_AFTER_MAPPER_INIT(261);
	EXTCL_CPU_WR_MEM(261);
	EXTCL_SAVE_MAPPER(261);
	mapper.internal_struct[0] = (BYTE *)&m261;
	mapper.internal_struct_size[0] = sizeof(m261);

	if (info.reset) {
		memset(&m261, 0x00, sizeof(m261));
	}
}
void extcl_after_mapper_init_261(void) {
	prg_fix_261();
	chr_fix_261();
}
void extcl_cpu_wr_mem_261(WORD address, UNUSED(BYTE value)) {
	m261.reg = address;
	prg_fix_261();
	chr_fix_261();
}
BYTE extcl_save_mapper_261(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m261.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_261(void) {
	WORD bank = m261.reg >> 7;

	if (m261.reg & 0x0040) {
		memmap_auto_32k(MMCPU(0x8000), bank);
	} else {
		bank = (bank << 1) | ((m261.reg & 0x0020) >> 5);
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_261(void) {
	memmap_auto_8k(MMPPU(0x0000), (m261.reg & 0x0F));
}
