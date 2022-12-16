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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_0(void);

struct _m0tmp {
	BYTE nrom368;
	BYTE *prg_4000;
} m0tmp;

void map_init_0(void) {
	EXTCL_AFTER_MAPPER_INIT(0);
	EXTCL_CPU_WR_MEM(0);
	EXTCL_CPU_RD_MEM(0);
	EXTCL_SAVE_MAPPER(0);

	m0tmp.nrom368 = (info.prg.rom.banks_16k == 3);
}
void extcl_after_mapper_init_0(void) {
	prg_fix_0();
}
void extcl_cpu_wr_mem_0(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_0(WORD address, BYTE openbus, BYTE before) {
	if (address < 0x6000) {
		if (m0tmp.prg_4000) {
			return (m0tmp.prg_4000[address & 0x1FFF]);
		}
		return (before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_0(UNUSED(BYTE mode), UNUSED(BYTE slot), UNUSED(FILE *fp)) {
	if (mode == SAVE_SLOT_READ) {
		prg_fix_0();
	}
	return (EXIT_OK);
}

INLINE static void prg_fix_0(void) {
	if (m0tmp.nrom368) {
		m0tmp.prg_4000 = prg_pnt(0);
		map_prg_rom_8k(2, 0, 1);
		map_prg_rom_8k(2, 2, 2);
		map_prg_rom_8k_update();
	} else {
		m0tmp.prg_4000 = NULL;
	}
}
