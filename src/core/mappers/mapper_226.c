/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

INLINE static void prg_fix_226(void);
INLINE static void chr_fix_226(void);
INLINE static void mirroring_fix_226(void);

struct _m226 {
	BYTE reg[2];
} m226;

void map_init_226(void) {
	EXTCL_AFTER_MAPPER_INIT(226);
	EXTCL_CPU_WR_MEM(226);
	EXTCL_SAVE_MAPPER(226);
	map_internal_struct_init((BYTE *)&m226, sizeof(m226));

	m226.reg[0] = 0;
	m226.reg[1] = 0;
}
void extcl_after_mapper_init_226(void) {
	prg_fix_226();
	chr_fix_226();
	mirroring_fix_226();
}
void extcl_cpu_wr_mem_226(UNUSED(BYTE nidx), WORD address, BYTE value) {
	m226.reg[address & 0x0001] = value;
	prg_fix_226();
	mirroring_fix_226();
}
BYTE extcl_save_mapper_226(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m226.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_226(void) {
	WORD bank = (m226.reg[0] >> 7) | ((m226.reg[1] & 0x01) << 1);

	// bmcghostbusters63in1 : Mapper 226 with 1536 KiB: Outer bank order 0 0 1 2
	if ((prgrom_banks(S16K) == (1536 / 16)) && (bank > 0)) {
		bank--;
	}
	bank = (m226.reg[0] & 0x1F) | (bank << 5);

	if (m226.reg[0] & 0x20) {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		bank >>= 1;
		memmap_auto_32k(0, MMCPU(0x8000), bank);
	}
}
INLINE static void chr_fix_226(void) {
	memmap_vram_wp_8k(0, MMPPU(0x0000), 0, TRUE, !(m226.reg[1] & 0x02));
}
INLINE static void mirroring_fix_226(void) {
	if (m226.reg[0] & 0x40) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}