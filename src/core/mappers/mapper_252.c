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

void prg_swap_vrc2and4_252(WORD address, WORD value);
void chr_swap_vrc2and4_252(WORD address, WORD value);

struct _m252 {
	WORD mask;
	WORD compare;
} m252;

void map_init_252(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(VRC2and4);
	EXTCL_CPU_RD_MEM(VRC2and4);
	EXTCL_SAVE_MAPPER(252);
	EXTCL_WR_CHR(252);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m252;
	mapper.internal_struct_size[0] = sizeof(m252);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	if (info.reset >= HARD) {
		memset(&m252, 0x00, sizeof(m252));
		if (info.mapper.id == 252) {
			m252.mask = 0xFE;
			m252.compare = 0x06;
		} else {
			m252.mask = 0xFE;
			m252.compare = 0x04;
		}
	}

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE, info.reset);
	VRC2and4_prg_swap = prg_swap_vrc2and4_252;
	VRC2and4_chr_swap = chr_swap_vrc2and4_252;
}
BYTE extcl_save_mapper_252(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m252.mask);
	save_slot_ele(mode, slot, m252.compare);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
void extcl_wr_chr_252(BYTE nidx, WORD address, UNUSED(BYTE value)) {
	switch (vrc2and4.chr[address >> 10]) {
		case 0x88:
			m252.mask = 0xFC;
			m252.compare = 0x4C;
			break;
		case 0xC2:
			m252.mask = 0xFE;
			m252.compare = 0x7C;
			break;
		case 0xC8:
			m252.mask = 0xFE;
			m252.compare = 0x04;
			break;
	}
	chr_wr(nidx, address, value);
}

void prg_swap_vrc2and4_252(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_252(WORD address, WORD value) {
	if (((value & m252.mask) == m252.compare) && vram_size(0)) {
		memmap_vram_1k(0, MMPPU(address), value);
	} else {
		chr_swap_VRC2and4_base(address, value);
	}
}
