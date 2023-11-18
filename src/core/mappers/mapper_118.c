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

void mirroring_fix_mmc3_118(void);

void map_init_118(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(118);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	init_MMC3(info.reset);
	MMC3_mirroring_fix = mirroring_fix_mmc3_118;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_118(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8001:
			switch (mmc3.bank_to_update & 0x07) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					mmc3.reg[mmc3.bank_to_update & 0x07] = value;
					MMC3_chr_fix();
					MMC3_mirroring_fix();
					break;
				default:
					extcl_cpu_wr_mem_MMC3(nidx, address, value);
					break;
			}
			return;
		case 0xA000:
			return;
	}
	extcl_cpu_wr_mem_MMC3(nidx, address, value);
}

void mirroring_fix_mmc3_118(void) {
	if (mmc3.bank_to_update & 0x80) {
		memmap_nmt_1k(0, MMPPU(0x2000), ((mmc3.reg[2] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x2400), ((mmc3.reg[3] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x2800), ((mmc3.reg[4] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x2C00), ((mmc3.reg[5] >> 7) ^ 0x01));

		memmap_nmt_1k(0, MMPPU(0x3000), ((mmc3.reg[2] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x3400), ((mmc3.reg[3] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x3800), ((mmc3.reg[4] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x3C00), ((mmc3.reg[5] >> 7) ^ 0x01));
	} else {
		memmap_nmt_1k(0, MMPPU(0x2000), ((mmc3.reg[0] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x2400), ((mmc3.reg[0] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x2800), ((mmc3.reg[1] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x2C00), ((mmc3.reg[1] >> 7) ^ 0x01));

		memmap_nmt_1k(0, MMPPU(0x3000), ((mmc3.reg[0] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x3400), ((mmc3.reg[0] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x3800), ((mmc3.reg[1] >> 7) ^ 0x01));
		memmap_nmt_1k(0, MMPPU(0x3C00), ((mmc3.reg[1] >> 7) ^ 0x01));
	}
}
