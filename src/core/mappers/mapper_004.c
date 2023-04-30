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

void map_init_004(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(MMC3);
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
		init_MMC3();
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	switch (info.mapper.submapper) {
		default:
		case DEFAULT:
			info.mapper.submapper = MMC3_SHARP;
			break;
		case MMC3_NEC:
			EXTCL_IRQ_A12_CLOCK(MMC3_NEC);
			break;
	}

	if (info.id == SMB2JSMB1) {
		info.prg.ram.banks_8k_plus = 1;
	}

	if (info.id == SMB2EREZA) {
		info.prg.ram.bat.banks = FALSE;
	}

	if (info.id == RADRACER2) {
		mirroring_FSCR();
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
