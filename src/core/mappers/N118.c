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

void (*N118_prg_fix)(void);
void (*N118_prg_swap)(WORD address, WORD value);
void (*N118_chr_fix)(void);
void (*N118_chr_swap)(WORD address, WORD value);

_n118 n118;

// promemoria
//void map_init_N118(void) {
//	EXTCL_AFTER_MAPPER_INIT(N118);
//	EXTCL_CPU_WR_MEM(N118);
//	EXTCL_SAVE_MAPPER(N118);
//}

void init_N118(void) {
	if (info.reset >= HARD) {
		memset(&n118, 0x00, sizeof(n118));

		n118.reg[0] = 0;
		n118.reg[1] = 2;
		n118.reg[2] = 4;
		n118.reg[3] = 5;
		n118.reg[4] = 6;
		n118.reg[5] = 7;
		n118.reg[6] = 0;
		n118.reg[7] = 1;

		n118.reg[8] = 0;
	}

	N118_prg_fix = prg_fix_N118_base;
	N118_prg_swap = prg_swap_N118_base;
	N118_chr_fix = chr_fix_N118_base;
	N118_chr_swap = chr_swap_N118_base;
}
void extcl_after_mapper_init_N118(void) {
	N118_prg_fix();
	N118_chr_fix();
}
void extcl_cpu_wr_mem_N118(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
			if (address & 0x0001) {
				n118.reg[n118.reg[8] & 0x07] = value;
			} else {
				n118.reg[8] = value;
			}
			N118_prg_fix();
			N118_chr_fix();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_N118(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, n118.reg);

	return (EXIT_OK);
}

void prg_fix_N118_base(void) {
	N118_prg_swap(0x8000, n118.reg[6]);
	N118_prg_swap(0xA000, n118.reg[7]);
	N118_prg_swap(0xC000, ~1);
	N118_prg_swap(0xE000, ~0);
}
void prg_swap_N118_base(WORD address, WORD value) {
	memmap_auto_8k(MMCPU(address), value);
}
void chr_fix_N118_base(void) {
	N118_chr_swap(0x0000, n118.reg[0] & (~1));
	N118_chr_swap(0x0400, n118.reg[0] |   1);
	N118_chr_swap(0x0800, n118.reg[1] & (~1));
	N118_chr_swap(0x0C00, n118.reg[1] |   1);
	N118_chr_swap(0x1000, n118.reg[2]);
	N118_chr_swap(0x1400, n118.reg[3]);
	N118_chr_swap(0x1800, n118.reg[4]);
	N118_chr_swap(0x1C00, n118.reg[5]);
}
void chr_swap_N118_base(WORD address, WORD value) {
	memmap_auto_1k(MMPPU(address), value);
}
