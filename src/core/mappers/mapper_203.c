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

INLINE static void prg_fix_203(void);
INLINE static void chr_fix_203(void);

struct _m203 {
	BYTE reg;
} m203;

void map_init_203(void) {
	EXTCL_AFTER_MAPPER_INIT(203);
	EXTCL_CPU_WR_MEM(203);
	EXTCL_SAVE_MAPPER(203);

	if (info.reset >= HARD) {
		memset(&m203, 0x00, sizeof(m203));
	}
}
void extcl_after_mapper_init_203(void) {
	prg_fix_203();
	chr_fix_203();
}
void extcl_cpu_wr_mem_203(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	m203.reg = value;
	prg_fix_203();
	chr_fix_203();
}
BYTE extcl_save_mapper_203(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m203.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_203(void) {
	WORD bank = m203.reg >> 2;

	memmap_auto_16k(0, MMCPU(0x8000), bank);
	memmap_auto_16k(0, MMCPU(0xC000), bank);
}
INLINE static void chr_fix_203(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m203.reg & 0x03));
}
