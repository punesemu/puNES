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
#include "irqA12.h"
#include "save_slot.h"

struct _m238 {
	BYTE reg;
} m238;

void map_init_238(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(238);
	EXTCL_CPU_RD_MEM(238);
	EXTCL_SAVE_MAPPER(238);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m238;
	mapper.internal_struct_size[0] = sizeof(m238);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m238, 0x00, sizeof(m238));

	init_MMC3();

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_238(WORD address, BYTE value) {
	if ((address >= 0x4020) && (address <= 0x7FFF)) {
		static const BYTE m238_vlu[4] = { 0x00, 0x02, 0x02, 0x03 };

		m238.reg = m238_vlu[value & 0x03];
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}
}
BYTE extcl_cpu_rd_mem_238(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4020) && (address <= 0x7FFF)) {
		return (m238.reg);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_238(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m238.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}
