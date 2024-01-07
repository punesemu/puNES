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

#include "mappers.h"
#include "info.h"

void prg_swap_mmc2_009(WORD address, WORD value);
void chr_swap_mmc2_009(WORD address, WORD value);

void map_init_009(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC2);
	EXTCL_CPU_WR_MEM(MMC2);
	EXTCL_SAVE_MAPPER(MMC2);
	EXTCL_AFTER_RD_CHR(MMC2);
	EXTCL_UPDATE_R2006(MMC2);
	map_internal_struct_init((BYTE *)&mmc2, sizeof(mmc2));

	init_MMC2(info.reset);
	MMC2_prg_swap = prg_swap_mmc2_009;
	MMC2_chr_swap = chr_swap_mmc2_009;
}

void prg_swap_mmc2_009(WORD address, WORD value) {
	prg_swap_MMC2_base(address, (value & 0x0F));
}
void chr_swap_mmc2_009(WORD address, WORD value) {
	chr_swap_MMC2_base(address, (value & 0x1F));
}
