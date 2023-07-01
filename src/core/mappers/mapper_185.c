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

INLINE static void prg_fix_185(void);
INLINE static void chr_fix_185(void);

struct _m185 {
	BYTE reg;
	BYTE ppu_read_count;
} m185;

void map_init_185() {
	EXTCL_AFTER_MAPPER_INIT(185);
	EXTCL_CPU_WR_MEM(185);
	EXTCL_SAVE_MAPPER(185);
	mapper.internal_struct[0] = (BYTE *)&m185;
	mapper.internal_struct_size[0] = sizeof(m185);

	if ((info.mapper.submapper & 0x0C) != 0x04) {
		EXTCL_RD_R2007(185);
	}

	if (info.reset >= HARD) {
		memset(&m185, 0x00, sizeof(m185));
	}
}
void extcl_after_mapper_init_185(void) {
	prg_fix_185();
	chr_fix_185();
}
void extcl_cpu_wr_mem_185(UNUSED(WORD address), BYTE value) {
	m185.reg = value;
	chr_fix_185();
}
BYTE extcl_save_mapper_185(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m185.reg);
	save_slot_ele(mode, slot, m185.ppu_read_count);

	return (EXIT_OK);
}
void extcl_rd_r2007_185(void) {
	if (m185.ppu_read_count < 2) {
		m185.ppu_read_count++;
		chr_fix_185();
	}
}

INLINE static void prg_fix_185(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
INLINE static void chr_fix_185(void) {
	BYTE enabled = TRUE;

	if ((info.mapper.submapper & 0x0C) == 0x04) {
		enabled = (m185.reg & 0x03) == (info.mapper.submapper & 0x03);
	} else {
		enabled = (m185.ppu_read_count >= 2);
	}
	memmap_auto_wp_8k(MMPPU(0x0000), 0, enabled, enabled);
}
