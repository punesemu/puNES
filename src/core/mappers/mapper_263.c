/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

void map_init_263(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(263);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	init_MMC3(info.reset);

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_263(BYTE nidx, WORD address, BYTE value) {
#define kof97_fix_value() value = (value & 0xD8) | ((value & 0x20) >> 4) |\
	((value & 0x04) << 3) | ((value & 0x02) >> 1) | ((value & 0x01) << 2)

	if (address == 0x9000) {
		address = 0x8001;
	} else if (address == 0xA000) {
		kof97_fix_value();
	} else if (address == 0xD000) {
		address = 0xC001;
	} else if (address == 0xF000) {
		address = 0xE001;
	}

	switch (address & 0xF001) {
		case 0x8000:
		case 0x8001:
		case 0x9000:
		case 0x9001:
		case 0xC000:
		case 0xC001:
		case 0xD000:
		case 0xD001:
		case 0xE000:
		case 0xE001:
			kof97_fix_value();
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			return;
		default:
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			return;
	}

#undef kof97_fix_value
}
