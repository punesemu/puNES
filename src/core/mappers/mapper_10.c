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

#include "mappers.h"
#include "info.h"

void prg_swap_mmc4_10(WORD address, WORD value);
void chr_swap_mmc4_10(WORD address, WORD value);

void map_init_10(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC4);
	EXTCL_CPU_WR_MEM(MMC4);
	EXTCL_SAVE_MAPPER(MMC4);
	EXTCL_AFTER_RD_CHR(MMC4);
	EXTCL_UPDATE_R2006(MMC4);
	mapper.internal_struct[0] = (BYTE *)&mmc4;
	mapper.internal_struct_size[0] = sizeof(mmc4);

	if (info.reset >= HARD) {
		init_MMC4();
		MMC4_prg_swap = prg_swap_mmc4_10;
		MMC4_chr_swap = chr_swap_mmc4_10;
	}
}

void prg_swap_mmc4_10(WORD address, WORD value) {
	prg_swap_MMC4_base(address, (value & 0x0F));
}
void chr_swap_mmc4_10(WORD address, WORD value) {
	chr_swap_MMC4_base(address, (value & 0x1F));
}
