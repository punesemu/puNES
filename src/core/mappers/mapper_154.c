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

void prg_swap_n118_154(WORD address, WORD value);
void chr_fix_n118_154(void);

INLINE static void mirroring_fix_154(void);

struct _m154 {
	BYTE reg;
} m154;

void map_init_154(void) {
	EXTCL_AFTER_MAPPER_INIT(154);
	EXTCL_CPU_WR_MEM(154);
	EXTCL_SAVE_MAPPER(154);
	map_internal_struct_init((BYTE *)&m154, sizeof(m154));
	map_internal_struct_init((BYTE *)&n118, sizeof(n118));

	if (info.reset >= HARD) {
		memset(&m154, 0x00, sizeof(m154));
	}

	init_N118(info.reset);
	N118_prg_swap = prg_swap_n118_154;
	N118_chr_fix = chr_fix_n118_154;
}
void extcl_after_mapper_init_154(void) {
	extcl_after_mapper_init_N118();
	mirroring_fix_154();
}
void extcl_cpu_wr_mem_154(BYTE nidx, WORD address, BYTE value) {
	if (address <= 0x9FFF) {
		extcl_cpu_wr_mem_N118(nidx, address, value);
	}
	m154.reg = value;
	mirroring_fix_154();
}
BYTE extcl_save_mapper_154(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, n118.reg);
	return (extcl_save_mapper_N118(mode, slot, fp));
}

void prg_swap_n118_154(WORD address, WORD value) {
	prg_swap_N118_base(address, (value & 0x0F));
}
void chr_fix_n118_154(void) {
	memmap_auto_2k(0, MMPPU(0x0000), ((n118.reg[0] & 0x3E) >> 1));
	memmap_auto_2k(0, MMPPU(0x0800), ((n118.reg[1] & 0x3E) >> 1));
	memmap_auto_1k(0, MMPPU(0x1000), (0x40 | n118.reg[2]));
	memmap_auto_1k(0, MMPPU(0x1400), (0x40 | n118.reg[3]));
	memmap_auto_1k(0, MMPPU(0x1800), (0x40 | n118.reg[4]));
	memmap_auto_1k(0, MMPPU(0x1C00), (0x40 | n118.reg[5]));
}

INLINE static void mirroring_fix_154(void) {
	if (m154.reg & 0x40) {
		mirroring_SCR1(0);
	} else {
		mirroring_SCR0(0);
	}
}