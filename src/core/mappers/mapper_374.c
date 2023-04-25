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
#include "mem_map.h"
#include "save_slot.h"

void prg_swap_374(WORD address, WORD value);
void chr_swap_374(WORD address, WORD value);

struct _m374 {
	BYTE reg;
} m374;

void map_init_374(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_SAVE_MAPPER(374);
	mapper.internal_struct[0] = (BYTE *)&m374;
	mapper.internal_struct_size[0] = sizeof(m374);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	init_MMC1(MMC1A);
	MMC1_prg_swap = prg_swap_374;
	MMC1_chr_swap = chr_swap_374;

	if (info.reset == RESET) {
		m374.reg++;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m374.reg = 0;
	}
}
BYTE extcl_save_mapper_374(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m374.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_374(WORD address, WORD value) {
	prg_swap_MMC1(address, ((m374.reg << 3) | (value & 0x07)));
}
void chr_swap_374(WORD address, WORD value) {
	chr_swap_MMC1(address, ((m374.reg << 5) | (value & 0x1F)));
}
