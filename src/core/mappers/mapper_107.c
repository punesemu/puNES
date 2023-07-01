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

INLINE static void prg_fix_107(void);
INLINE static void chr_fix_107(void);

struct _m107 {
	WORD reg;
} m107;

void map_init_107(void) {
	EXTCL_AFTER_MAPPER_INIT(107);
	EXTCL_CPU_WR_MEM(107);
	EXTCL_SAVE_MAPPER(107);
	mapper.internal_struct[0] = (BYTE *)&m107;
	mapper.internal_struct_size[0] = sizeof(m107);

	if (info.reset >= HARD) {
		memset(&m107, 0x00, sizeof(m107));
	}
}
void extcl_after_mapper_init_107(void) {
	prg_fix_107();
	chr_fix_107();
}
void extcl_cpu_wr_mem_107(WORD address, UNUSED(BYTE value)) {
	m107.reg = address;
	prg_fix_107();
	chr_fix_107();
}
BYTE extcl_save_mapper_107(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m107.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_107(void) {
	memmap_auto_32k(MMCPU(0x8000), (m107.reg >> 1));
}
INLINE static void chr_fix_107(void) {
	memmap_auto_8k(MMPPU(0x0000), m107.reg);
}
