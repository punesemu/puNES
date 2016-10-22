/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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
#include "irqA12.h"
#include "save_slot.h"

static const BYTE unif603_5052_vlu[4] = { 0x00, 0x02, 0x02, 0x03 };

void map_init_UNIF603_5052(void) {
	EXTCL_CPU_WR_MEM(UNIF603_5052);
	EXTCL_CPU_RD_MEM(UNIF603_5052);
	EXTCL_SAVE_MAPPER(UNIF603_5052);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_UNIF603_5052(WORD address, BYTE value) {
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	if ((address >= 0x4020) && (address <= 0x7FFF)) {
		unif603_5052.reg = unif603_5052_vlu[value & 0x03];
		return;
	}
}
BYTE extcl_cpu_rd_mem_UNIF603_5052(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x4020) && (address <= 0x7FFF)) {
		return (unif603_5052.reg);
	}
	return (openbus);
}
BYTE extcl_save_mapper_UNIF603_5052(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unif603_5052.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
