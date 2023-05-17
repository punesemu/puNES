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
#include "mem_map.h"
#include "info.h"
#include "save_slot.h"

INLINE static void prg_fix_171(void);
INLINE static void wram_fix_171(void);

struct _m171 {
	BYTE reg[2];
} m171;

void map_init_171(void) {
	EXTCL_AFTER_MAPPER_INIT(171);
	EXTCL_CPU_WR_MEM(171);
	EXTCL_SAVE_MAPPER(171);

	if (info.reset >= HARD) {
		memset(&m171, 0x00, sizeof(m171));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_171(void) {
	prg_fix_171();
	wram_fix_171();
}
void extcl_cpu_wr_mem_171(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		m171.reg[address & 0x01] = value;
		wram_fix_171();
	}
}
BYTE extcl_save_mapper_171(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m171.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_171(void) {
	memmap_auto_32k(0x8000, 0);
}
INLINE static void wram_fix_171(void) {
	memmap_prgrom_4k(0x6000, m171.reg[0]);
	memmap_prgrom_4k(0x7000, m171.reg[1]);
}
