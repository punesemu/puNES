/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#define kof97_fix_value() value = (value & 0xD8) | ((value & 0x20) >> 4) |\
	((value & 0x04) << 3) | ((value & 0x02) >> 1) | ((value & 0x01) << 2)

void map_init_KOF97(void) {
	EXTCL_CPU_WR_MEM(KOF97);
	EXTCL_SAVE_MAPPER(KOF97);
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

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_KOF97(WORD address, BYTE value) {
	if (address == 0x9000) {
		address = 0x8001;
	} else if (address == 0xA000) {
		address = 0x8000;
	} else if (address == 0xD000) {
		address = 0xC001;
	} else if (address == 0xF000) {
		address = 0xE001;
	}

	switch (address & 0xE001) {
		case 0x8000:
		case 0x8001:
		case 0xC000:
		case 0xC001:
		case 0xE000:
		case 0xE001:
			kof97_fix_value();
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
		default:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_save_mapper_KOF97(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
