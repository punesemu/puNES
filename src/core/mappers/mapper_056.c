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

void prg_swap_ks202_056(WORD address, WORD value);
void wram_swap_ks202_056(WORD address, WORD value);

INLINE static void chr_fix_056(void);
INLINE static void mirroring_fix_056(void);

struct _m056 {
	BYTE prg[4];
	BYTE chr[8];
	BYTE mirroring;
} m056;

void map_init_056(void) {
	EXTCL_AFTER_MAPPER_INIT(056);
	EXTCL_CPU_WR_MEM(056);
	EXTCL_SAVE_MAPPER(056);
	EXTCL_CPU_EVERY_CYCLE(KS202);
	mapper.internal_struct[0] = (BYTE *)&m056;
	mapper.internal_struct_size[0] = sizeof(m056);
	mapper.internal_struct[1] = (BYTE *)&ks202;
	mapper.internal_struct_size[1] = sizeof(ks202);

	memset(&m056, 0x00, sizeof(m056));

	m056.prg[0] = 0x10;
	m056.prg[1] = 0x10;
	m056.prg[2] = 0x10;
	m056.prg[3] = 0x10;

	init_KS202();
	KS202_prg_swap = prg_swap_ks202_056;
	KS202_wram_swap = wram_swap_ks202_056;
}
void extcl_after_mapper_init_056(void) {
	extcl_after_mapper_init_KS202();
	chr_fix_056();
	mirroring_fix_056();
}
void extcl_cpu_wr_mem_056(WORD address, BYTE value) {
	extcl_cpu_wr_mem_KS202(address, value);
	if (address >= 0xF000) {
		switch ((address & 0x0FFF) >> 10) {
			case 0:
				m056.prg[address & 0x03] = value & 0x10;
				break;
			case 2:
				m056.mirroring = value;
				break;
			case 3:
				m056.chr[address & 0x07] = value;
				break;
		}
		KS202_prg_fix();
		KS202_wram_fix();
		chr_fix_056();
	}
}
BYTE extcl_save_mapper_056(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m056.prg);
	save_slot_ele(mode, slot, m056.chr);
	save_slot_ele(mode, slot, m056.mirroring);
	return (extcl_save_mapper_KS202(mode, slot, fp));
}

void prg_swap_ks202_056(WORD address, WORD value) {
	const BYTE slot = (address >> 13) & 0x03;
	WORD base = slot == 3 ? 0x10 : m056.prg[slot];

	prg_swap_KS202_base(address, (base | (value & 0x0F)));
}
void wram_swap_ks202_056(WORD address, WORD value) {
	wram_swap_KS202_base(address, (value & 0x0F));
}

INLINE static void chr_fix_056(void) {
	memmap_auto_1k(MMPPU(0x0000), m056.chr[0]);
	memmap_auto_1k(MMPPU(0x0400), m056.chr[1]);
	memmap_auto_1k(MMPPU(0x0800), m056.chr[2]);
	memmap_auto_1k(MMPPU(0x0C00), m056.chr[3]);
	memmap_auto_1k(MMPPU(0x1000), m056.chr[4]);
	memmap_auto_1k(MMPPU(0x1400), m056.chr[5]);
	memmap_auto_1k(MMPPU(0x1800), m056.chr[6]);
	memmap_auto_1k(MMPPU(0x1C00), m056.chr[7]);
}
INLINE static void mirroring_fix_056(void) {
	if (m056.mirroring & 0x01) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
