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
#include "mem_map.h"

void prg_swap_vrc2and4_302(WORD address, WORD value);
void chr_swap_vrc2and4_302(WORD address, WORD value);

void map_init_302(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(VRC2and4);
	EXTCL_CPU_RD_MEM(302);
	EXTCL_SAVE_MAPPER(VRC2and4);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_302;
	VRC2and4_chr_swap = chr_swap_vrc2and4_302;

	info.mapper.extend_rd = TRUE;
}
BYTE extcl_cpu_rd_mem_302(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x9FFF)) {
		BYTE reg = ((((address >> 12) - 0x06) << 1) | ((address & 0x0800) >> 11)) ^ 0x04;

		return (prg_byte((vrc2and4.chr[reg] << 11) | (address & 0x07FF)));
	}
	return (openbus);
}

void prg_swap_vrc2and4_302(WORD address, WORD value) {
	const WORD slot = (address >> 13) & 0x03;

	value = !slot ? 0 : 0x0D + (slot - 1);
	prg_swap_VRC2and4_base(address, value);
}
void chr_swap_vrc2and4_302(WORD address, UNUSED(WORD value)) {
	chr_swap_VRC2and4_base(address, (address >> 10));
}