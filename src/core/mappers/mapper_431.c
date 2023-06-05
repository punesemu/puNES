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

INLINE static void prg_fix_431(void);
INLINE static void mirroring_fix_431(void);

struct _m431 {
	BYTE reg[2];
} m431;

void map_init_431(void) {
	EXTCL_AFTER_MAPPER_INIT(431);
	EXTCL_CPU_WR_MEM(431);
	EXTCL_SAVE_MAPPER(431);
	mapper.internal_struct[0] = (BYTE *)&m431;
	mapper.internal_struct_size[0] = sizeof(m431);

	memset(&m431, 0x00, sizeof(m431));
}
void extcl_after_mapper_init_431(void) {
	prg_fix_431();
	mirroring_fix_431();
}
void extcl_cpu_wr_mem_431(WORD address, BYTE value) {
	// bus conflict
	value &= prgrom_rd(address);

	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			m431.reg[0] = value;
			prg_fix_431();
			mirroring_fix_431();
			break;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			m431.reg[1] = value;
			prg_fix_431();
			break;
	}
}
BYTE extcl_save_mapper_431(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m431.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_431(void) {
	WORD base = (m431.reg[0] & 0x20) >> 2;

	memmap_auto_16k(MMCPU(0x8000), (base | (m431.reg[1] & 0x07)));
	memmap_auto_16k(MMCPU(0xC000), (base | 0x07));
}
INLINE static void mirroring_fix_431(void) {
	if (m431.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
