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

INLINE static void prg_fix_214(void);
INLINE static void chr_fix_214(void);

struct _m214 {
	WORD reg;
} m214;

void map_init_214(void) {
	EXTCL_AFTER_MAPPER_INIT(214);
	EXTCL_CPU_WR_MEM(214);
	EXTCL_SAVE_MAPPER(214);
	mapper.internal_struct[0] = (BYTE *)&m214;
	mapper.internal_struct_size[0] = sizeof(m214);

	if (info.reset >= HARD) {
		memset(&m214, 0x00, sizeof(m214));
	}
}
void extcl_after_mapper_init_214(void) {
	prg_fix_214();
	chr_fix_214();
}
void extcl_cpu_wr_mem_214(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m214.reg = address;
	prg_fix_214();
	chr_fix_214();
}
BYTE extcl_save_mapper_214(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m214.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_214(void) {
	WORD bank = m214.reg >> 2;

	memmap_auto_16k(0, MMCPU(0x8000), bank);
	memmap_auto_16k(0, MMCPU(0xC000), bank);
}
INLINE static void chr_fix_214(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m214.reg >> 2));
}
