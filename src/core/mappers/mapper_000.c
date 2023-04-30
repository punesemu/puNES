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

INLINE static void prg_fix_000(void);

struct _m000tmp {
	BYTE nrom368;
	BYTE *prg_4000;
} m000tmp;

void map_init_000(void) {
	EXTCL_AFTER_MAPPER_INIT(000);
	EXTCL_CPU_INIT_PC(000);
	EXTCL_CPU_WR_MEM(000);
	EXTCL_CPU_RD_MEM(000);
	EXTCL_SAVE_MAPPER(000);

	m000tmp.nrom368 = (info.prg.rom.banks_16k == 3);
}
void extcl_after_mapper_init_000(void) {
	prg_fix_000();
}
void extcl_cpu_init_pc_000(void) {
	if (info.reset >= HARD) {
		if (info.mapper.trainer && prg.ram_plus_8k) {
			BYTE *data = &prg.ram_plus_8k[0x7000 & 0x1FFF];

			memcpy(data, &mapper.trainer[0], sizeof(mapper.trainer));
		}
	}
}
void extcl_cpu_wr_mem_000(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_000(WORD address, BYTE openbus, BYTE before) {
	if (address < 0x6000) {
		if (m000tmp.prg_4000) {
			return (m000tmp.prg_4000[address & 0x1FFF]);
		}
		return (before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_000(UNUSED(BYTE mode), UNUSED(BYTE slot), UNUSED(FILE *fp)) {
	if (mode == SAVE_SLOT_READ) {
		prg_fix_000();
	}
	return (EXIT_OK);
}

INLINE static void prg_fix_000(void) {
	if (m000tmp.nrom368) {
		m000tmp.prg_4000 = prg_pnt(0);
		map_prg_rom_8k(2, 0, 1);
		map_prg_rom_8k(2, 2, 2);
	} else {
		m000tmp.prg_4000 = NULL;
		map_prg_rom_8k(4, 0, 0);
	}
	map_prg_rom_8k_update();
}
