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

INLINE static void prg_fix_414(void);
INLINE static void chr_fix_414(void);
INLINE static void mirroring_fix_414(void);

struct _m414 {
	WORD reg[2];
} m414;

void map_init_414(void) {
	EXTCL_AFTER_MAPPER_INIT(414);
	EXTCL_CPU_WR_MEM(414);
	EXTCL_SAVE_MAPPER(414);
	mapper.internal_struct[0] = (BYTE *)&m414;
	mapper.internal_struct_size[0] = sizeof(m414);

	if (info.reset >= HARD) {
		memset(&m414, 0x00, sizeof(m414));
	}
}
void extcl_after_mapper_init_414(void) {
	prg_fix_414();
	chr_fix_414();
	mirroring_fix_414();
}
void extcl_cpu_wr_mem_414(WORD address, BYTE value) {
	m414.reg[0] = address;
	// bus conflict
	m414.reg[1] = value & prgrom_rd(address);
	prg_fix_414();
	chr_fix_414();
	mirroring_fix_414();
}
BYTE extcl_save_mapper_414(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m414.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_414(void) {
	WORD bank = m414.reg[0] >> 1;

	if (!(m414.reg[0] & 0x0100) && (m414.reg[0] & dipswitch.value)) {
		memmap_disable_16k(MMCPU(0xC000));
	} else if (m414.reg[0] & 0x2000) {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_414(void) {
	memmap_auto_8k(MMPPU(0x0000), m414.reg[1]);
}
INLINE static void mirroring_fix_414(void) {
	if (m414.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
