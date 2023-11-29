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
	map_internal_struct_init((BYTE *)&m238, sizeof(m238));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m238, 0x00, sizeof(m238));

	init_MMC3(info.reset);

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_238(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4020) && (address <= 0x7FFF)) {
		static const BYTE m238_vlu[4] = { 0x00, 0x02, 0x02, 0x03 };

		m238.reg = m238_vlu[value & 0x03];
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
		return;
	}
}
BYTE extcl_cpu_rd_mem_238(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4020) && (address <= 0x7FFF)) {
		return (m238.reg);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_238(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m238.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}
