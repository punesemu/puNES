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

#include "mappers.h"
#include "info.h"

void prg_fix_n118_307(void);
void chr_fix_n118_307(void);
void chr_swap_n118_307(WORD address, WORD value);

INLINE static void wram_fix_307(void);
INLINE static void mirroring_fix_307(void);

void map_init_307(void) {
	EXTCL_AFTER_MAPPER_INIT(307);
	EXTCL_CPU_WR_MEM(N118);
	EXTCL_SAVE_MAPPER(307);
	mapper.internal_struct[0] = (BYTE *)&n118;
	mapper.internal_struct_size[0] = sizeof(n118);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S4K);
	}

	init_N118(info.reset);
	N118_prg_fix = prg_fix_n118_307;
	N118_chr_fix = chr_fix_n118_307;
	N118_chr_swap = chr_swap_n118_307;
}
void extcl_after_mapper_init_307(void) {
	extcl_after_mapper_init_N118();
	wram_fix_307();
}
BYTE extcl_save_mapper_307(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_N118(mode, slot, fp);

	return (EXIT_OK);
}

void prg_fix_n118_307(void) {
	memmap_auto_8k(0, MMCPU(0x8000), n118.reg[6]);
	memmap_auto_4k(0, MMCPU(0xA000), 28);
	memmap_wram_4k(0, MMCPU(0xB000), 1);
	memmap_auto_8k(0, MMCPU(0xC000), n118.reg[7]);
	memmap_auto_8k(0, MMCPU(0xE000), 15);
}
void chr_fix_n118_307(void) {
	chr_fix_N118_base();
	mirroring_fix_307();
}
void chr_swap_n118_307(WORD address, UNUSED(WORD value)) {
	chr_swap_N118_base(address, ((address >> 10) & 0x07));
}

INLINE static void wram_fix_307(void) {
	memmap_auto_4k(0, MMCPU(0x6000), 0);
	memmap_prgrom_4k(0, MMCPU(0x7000), 15);
}
INLINE static void mirroring_fix_307(void) {
	memmap_nmt_1k(0, MMPPU(0x2000), (n118.reg[2] & 0x01));
	memmap_nmt_1k(0, MMPPU(0x2400), (n118.reg[4] & 0x01));
	memmap_nmt_1k(0, MMPPU(0x2800), (n118.reg[3] & 0x01));
	memmap_nmt_1k(0, MMPPU(0x2C00), (n118.reg[5] & 0x01));

	memmap_nmt_1k(0, MMPPU(0x3000), (n118.reg[2] & 0x01));
	memmap_nmt_1k(0, MMPPU(0x3400), (n118.reg[4] & 0x01));
	memmap_nmt_1k(0, MMPPU(0x3800), (n118.reg[3] & 0x01));
	memmap_nmt_1k(0, MMPPU(0x3C00), (n118.reg[5] & 0x01));
}
