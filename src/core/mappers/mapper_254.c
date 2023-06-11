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

struct _m254 {
	BYTE reg[2];
} m254;

void map_init_254(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(254);
	EXTCL_CPU_RD_MEM(254);
	EXTCL_SAVE_MAPPER(254);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m254;
	mapper.internal_struct_size[0] = sizeof(m254);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m254, 0x00, sizeof(m254));

	init_MMC3();

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_254(WORD address, BYTE value) {
	if (address == 0x8000) {
		m254.reg[0] = 0xFF;
	} else if (address == 0xA001) {
		m254.reg[1] = value;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_cpu_rd_mem_254(WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!m254.reg[0]) {
			return (openbus ^ m254.reg[1]);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_254(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m254.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}
