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

void prg_swap_lz93d50_153(WORD address, WORD value);
void chr_fix_lz93d50_153(void);

struct _m153 {
	BYTE outer;
} m153;

void map_init_153(void) {
	EXTCL_AFTER_MAPPER_INIT(LZ93D50);
	EXTCL_CPU_WR_MEM(153);
	EXTCL_CPU_RD_MEM(LZ93D50);
	EXTCL_SAVE_MAPPER(153);
	EXTCL_CPU_EVERY_CYCLE(LZ93D50);
	map_internal_struct_init((BYTE *)&m153, sizeof(m153));
	map_internal_struct_init((BYTE *)&lz93d50, sizeof(lz93d50));

	if (info.reset >= HARD) {
		memset(&m153, 0x00, sizeof(m153));
	}

	init_LZ93D50(FALSE, info.reset);
	LZ93D50_prg_swap = prg_swap_lz93d50_153;
	LZ93D50_chr_fix = chr_fix_lz93d50_153;
}
void extcl_cpu_wr_mem_153(BYTE nidx, WORD address, BYTE value) {
	if (address >= 0x8000) {
		if ((address & 0x0F) <= 0x03) {
			m153.outer = value;
			LZ93D50_prg_fix();
		}
	}
	extcl_cpu_wr_mem_LZ93D50(nidx, address, value);
}
BYTE extcl_save_mapper_153(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m153.outer);
	return (extcl_save_mapper_LZ93D50(mode, slot, fp));
}

void prg_swap_lz93d50_153(WORD address, WORD value) {
	prg_swap_LZ93D50_base(address, ((m153.outer << 4) | (value & 0x0F)));
}
void chr_fix_lz93d50_153(void) {
	memmap_auto_8k(0, MMPPU(0x0000), 0);
}
