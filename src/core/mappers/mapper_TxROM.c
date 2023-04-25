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
#include "irqA12.h"
#include "save_slot.h"

void mirroring_fix_TxROM(void);

void map_init_TxROM(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(TxROM);
	EXTCL_SAVE_MAPPER(MMC3);
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
	MMC3_mirroring_fix = mirroring_fix_TxROM;

	if ((info.format != NES_2_0) && (info.mapper.submapper == TKSROM)) {
		info.prg.ram.banks_8k_plus = 1;
		info.prg.ram.bat.banks = 1;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}

void extcl_cpu_wr_mem_TxROM(WORD address, BYTE value) {
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
					extcl_cpu_wr_mem_MMC3(address, value);
					break;
			}
			return;
		case 0xA000:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}

void mirroring_fix_TxROM(void) {
	if (mmc3.bank_to_update & 0x80) {
		ntbl.bank_1k[0] = &ntbl.data[((mmc3.reg[2] >> 7) ^ 0x01) << 10];
		ntbl.bank_1k[1] = &ntbl.data[((mmc3.reg[3] >> 7) ^ 0x01) << 10];
		ntbl.bank_1k[2] = &ntbl.data[((mmc3.reg[4] >> 7) ^ 0x01) << 10];
		ntbl.bank_1k[3] = &ntbl.data[((mmc3.reg[5] >> 7) ^ 0x01) << 10];
	} else {
		ntbl.bank_1k[0] = &ntbl.data[((mmc3.reg[0] >> 7) ^ 0x01) << 10];
		ntbl.bank_1k[1] = ntbl.bank_1k[0];
		ntbl.bank_1k[2] = &ntbl.data[((mmc3.reg[1] >> 7) ^ 0x01) << 10];
		ntbl.bank_1k[3] = ntbl.bank_1k[2];
	}
}
