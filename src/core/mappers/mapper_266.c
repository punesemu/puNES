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

void prg_swap_vrc2and4_266(WORD address, WORD value);
void chr_swap_vrc2and4_266(WORD address, WORD value);
void misc_03_vrc2and4_266(WORD address, BYTE value);

struct _m266 {
	BYTE reg;
	BYTE pcm;
} m266;

void map_init_266(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(266);
	EXTCL_SAVE_MAPPER(266);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m266;
	mapper.internal_struct_size[0] = sizeof(m266);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	// TODO: aggiungere il suono

	memset(&m266, 0x00, sizeof(m266));

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_266;
	VRC2and4_chr_swap = chr_swap_vrc2and4_266;
	VRC2and4_misc_03 = misc_03_vrc2and4_266;

	m266.pcm = 7;
}
void extcl_cpu_wr_mem_266(WORD address, BYTE value) {
	address = (address & 0x9FFF) | ((address & 0x2000) << 1) | ((address & 0x4000) >> 1);
	extcl_cpu_wr_mem_VRC2and4(address, value);
}
BYTE extcl_save_mapper_266(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m266.reg);
	save_slot_ele(mode, slot, m266.pcm);
	extcl_save_mapper_VRC2and4(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_vrc2and4_266(WORD address, WORD value) {
	value = (m266.reg << 2) | ((address >> 13) & 0x03);
	prg_swap_VRC2and4_base(address, value);
}
void chr_swap_vrc2and4_266(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}
void misc_03_vrc2and4_266(WORD address, BYTE value) {
	if (address & 0x0800) {
		m266.pcm = value & 0x0F;
	} else {
		m266.reg = value >> 2;
		VRC2and4_prg_fix();
	}
}