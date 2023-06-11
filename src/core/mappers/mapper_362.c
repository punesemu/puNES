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

void prg_swap_vrc2and4_362(WORD address, WORD value);
void chr_swap_vrc2and4_362(WORD address, WORD value);

struct _m362 {
	BYTE reg;
	BYTE game;
} m362;

void map_init_362(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(362);
	EXTCL_SAVE_MAPPER(362);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	EXTCL_RD_CHR(362);
	mapper.internal_struct[0] = (BYTE *)&m362;
	mapper.internal_struct_size[0] = sizeof(m362);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	init_VRC2and4(VRC24_VRC4, 0x01, 0x02, FALSE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_362;
	VRC2and4_chr_swap = chr_swap_vrc2and4_362;

	if ((info.reset >= HARD) || (prgrom_size() <= S512K)) {
		memset(&m362, 0x00, sizeof(m362));
		m362.game = 0;
	} else if (info.reset == RESET) {
		m362.game ^= 1;
	}
}
void extcl_cpu_wr_mem_362(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			VRC2and4_prg_fix();
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			return;
	}
}
BYTE extcl_save_mapper_362(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m362.reg);
	save_slot_ele(mode, slot, m362.game);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
BYTE extcl_rd_chr_362(WORD address) {
	BYTE reg = address >> 10;

	if (m362.reg != reg) {
		m362.reg = reg;
		VRC2and4_prg_fix();
		VRC2and4_chr_fix();
	}
	return (chr_rd(address));
}

void prg_swap_vrc2and4_362(WORD address, WORD value) {
	WORD base = 0x40;
	WORD mask = 0x0F;

	if (!m362.game) {
		base = (vrc2and4.chr[m362.reg] & 0x180) >> 3;
	}
	prg_swap_VRC2and4_base(address, (base | (value & mask)));
}
void chr_swap_vrc2and4_362(WORD address, WORD value) {
	WORD base = 0x200;
	WORD mask = 0x1FF;

	if (!m362.game) {
		base = vrc2and4.chr[m362.reg] & 0x180;
		mask = 0x7F;
	}
	chr_swap_VRC2and4_base(address, (base | (value & mask)));
}
