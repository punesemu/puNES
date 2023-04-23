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
#include <stdlib.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

struct _m198tmp {
	BYTE *prg_5000;
} m198tmp;

void map_init_198(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_MAPPER_QUIT(198);
	EXTCL_CPU_WR_MEM(198);
	EXTCL_CPU_RD_MEM(198);
	EXTCL_SAVE_MAPPER(198);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m198tmp.prg_5000 = (BYTE *)malloc(0x1000);
		memset(m198tmp.prg_5000, 0x00, 0x1000);
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_mapper_quit_198(void) {
	if (m198tmp.prg_5000) {
		free(m198tmp.prg_5000);
		m198tmp.prg_5000 = NULL;
	}
}
void extcl_cpu_wr_mem_198(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m198tmp.prg_5000[address & 0x0FFF] = value;
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_198(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (m198tmp.prg_5000[address & 0x0FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_198(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_mem(mode, slot, m198tmp.prg_5000, 0x1000, FALSE);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
