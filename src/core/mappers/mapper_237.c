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

INLINE static void prg_fix_237(void);
INLINE static void mirroring_fix_237(void);

struct _m237 {
	WORD reg[2];
} m237;

void map_init_237(void) {
	EXTCL_AFTER_MAPPER_INIT(237);
	EXTCL_CPU_WR_MEM(237);
	EXTCL_CPU_RD_MEM(237);
	EXTCL_SAVE_MAPPER(237);
	mapper.internal_struct[0] = (BYTE *)&m237;
	mapper.internal_struct_size[0] = sizeof(m237);

	memset(&m237, 0x00, sizeof(m237));

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_237(void) {
	prg_fix_237();
	mirroring_fix_237();
}
void extcl_cpu_wr_mem_237(WORD address, BYTE value) {
	if (m237.reg[0] & 0x02) {
		m237.reg[1] = (m237.reg[1] & 0xF8) | (value & 0x07);
	} else {
		m237.reg[0] = address;
		m237.reg[1] = value;
	}
	prg_fix_237();
	mirroring_fix_237();
}
BYTE extcl_cpu_rd_mem_237(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m237.reg[0] & 0x01 ? dipswitch.value : prgrom_rd(address));
	}
	return (wram_rd(address));
}

BYTE extcl_save_mapper_237(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m237.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_237(void) {
	WORD base = ((m237.reg[0] & 0x04) << 3) | (m237.reg[1] & 0x18);
	WORD bank[2];

	switch (m237.reg[1] & 0xC0) {
		default:
		case 0x00:
			bank[0] = base | (m237.reg[1] & 0x07);
			bank[1] = base | 0x07;
			break;
		case 0x40:
			bank[0] = base | (m237.reg[1] & 0x06);
			bank[1] = base | 0x07;
			break;
		case 0x80:
			bank[0] = bank[1] = base | (m237.reg[1] & 0x07);
			break;
		case 0xC0:
			bank[0] = base | (m237.reg[1] & 0x06);
			bank[1] = base | (m237.reg[1] & 0x06) | 0x01;
			break;
	}
	memmap_auto_16k(MMCPU(0x8000), bank[0]);
	memmap_auto_16k(MMCPU(0xC000), bank[1]);
}
INLINE static void mirroring_fix_237(void) {
	if (m237.reg[1] & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
