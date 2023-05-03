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
#include "mem_map.h"
#include "save_slot.h"

void prg_swap_n118v2_154(WORD address, WORD value);
void chr_fix_n118v2_154(void);

INLINE static void mirroring_fix_154(void);

struct _m154 {
	BYTE reg;
} m154;

void map_init_154(void) {
	EXTCL_AFTER_MAPPER_INIT(154);
	EXTCL_CPU_WR_MEM(154);
	EXTCL_SAVE_MAPPER(154);
	mapper.internal_struct[0] = (BYTE *)&m154;
	mapper.internal_struct_size[0] = sizeof(m154);
	mapper.internal_struct[1] = (BYTE *)&n118v2;
	mapper.internal_struct_size[1] = sizeof(n118v2);

	if (info.reset >= HARD) {
		memset(&n118v2, 0x00, sizeof(n118v2));
	}

	init_N118v2();
	N118v2_prg_swap = prg_swap_n118v2_154;
	N118v2_chr_fix = chr_fix_n118v2_154;
}
void extcl_after_mapper_init_154(void) {
	extcl_after_mapper_init_N118v2();
	mirroring_fix_154();
}
void extcl_cpu_wr_mem_154(WORD address, BYTE value) {
	if (address <= 0x9FFF) {
		extcl_cpu_wr_mem_N118v2(address, value);
	}
	m154.reg = value;
	mirroring_fix_154();
}
BYTE extcl_save_mapper_154(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, n118v2.reg);
	extcl_save_mapper_N118v2(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_n118v2_154(WORD address, WORD value) {
	prg_swap_N118v2_base(address, (value & 0x0F));
}
void chr_fix_n118v2_154(void) {
	map_chr_rom_2k(0x0000, ((n118v2.reg[0] & 0x3E) >> 1));
	map_chr_rom_2k(0x0800, ((n118v2.reg[1] & 0x3E) >> 1));
	map_chr_rom_1k(0x1000, (0x40 | n118v2.reg[2]));
	map_chr_rom_1k(0x1400, (0x40 | n118v2.reg[3]));
	map_chr_rom_1k(0x1800, (0x40 | n118v2.reg[4]));
	map_chr_rom_1k(0x1C00, (0x40 | n118v2.reg[5]));
}

INLINE static void mirroring_fix_154(void) {
	if (m154.reg & 0x40) {
		mirroring_SCR1();
	} else {
		mirroring_SCR0();
	}
}