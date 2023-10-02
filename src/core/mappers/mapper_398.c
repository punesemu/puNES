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
#include "save_slot.h"

void prg_swap_vrc2and4_398(WORD address, WORD value);
void chr_swap_vrc2and4_398(WORD address, WORD value);

struct _m398 {
	BYTE reg[2];
} m398;

void map_init_398(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(398);
	EXTCL_SAVE_MAPPER(398);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	EXTCL_RD_CHR(398);
	mapper.internal_struct[0] = (BYTE *)&m398;
	mapper.internal_struct_size[0] = sizeof(m398);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	memset(&m398, 0x00, sizeof(m398));

	m398.reg[0] = 0xC0;

	init_VRC2and4(VRC24_VRC4, 0x01, 0x02, TRUE, info.reset);
	VRC2and4_prg_swap = prg_swap_vrc2and4_398;
	VRC2and4_chr_swap = chr_swap_vrc2and4_398;
}
void extcl_cpu_wr_mem_398(BYTE nidx, WORD address, BYTE value) {
	BYTE reg0 = address & 0xFF;

	if (reg0 != m398.reg[0]) {
		m398.reg[0] = reg0;
		VRC2and4_prg_fix();
		VRC2and4_chr_fix();
	}
	extcl_cpu_wr_mem_VRC2and4(nidx, address, value);
}
BYTE extcl_save_mapper_398(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m398.reg);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
BYTE extcl_rd_chr_398(BYTE nidx, WORD address) {
	BYTE reg1 = address >> 10;

	if (reg1 != m398.reg[1]) {
		m398.reg[1] = reg1;
		VRC2and4_prg_fix();
		VRC2and4_chr_fix();
	}
	return (chr_rd(nidx, address));
}

void prg_swap_vrc2and4_398(WORD address, WORD value) {
	WORD mask = 0x0F;

	if (m398.reg[0] & 0x80) {
		value = ((((m398.reg[0] & 0xC0) >> 5) | ((vrc2and4.chr[m398.reg[1]] & 0x04) >> 2)) << 2) | ((address >> 13) & 0x03);
		mask = ~0;
	}
	prg_swap_VRC2and4_base(address, (value & mask));
}
void chr_swap_vrc2and4_398(WORD address, WORD value) {
	WORD mask = 0x1FF;

	if (m398.reg[0] & 0x80) {
		value = ((0x40 | ((m398.reg[0] & 0x40) >> 3) | ((vrc2and4.chr[m398.reg[1]] & 0x07))) << 3) | (address >> 10);
		mask = ~0;
	}
	chr_swap_VRC2and4_base(address, (value & mask));
}
