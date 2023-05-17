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
#include "save_slot.h"

void prg_fix_vrc2and4_183(void);
void chr_swap_vrc2and4_183(WORD address, WORD value);
void wram_fix_vrc2and4_183(void);

struct _m183 {
	BYTE reg[4];
} m183;

void map_init_183(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(183);
	EXTCL_SAVE_MAPPER(183);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m183;
	mapper.internal_struct_size[0] = sizeof(m183);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	if (info.reset >= HARD) {
		memset(&m183, 0x00, sizeof(m183));
	}

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE);
	VRC2and4_prg_fix = prg_fix_vrc2and4_183;
	VRC2and4_chr_swap = chr_swap_vrc2and4_183;
	VRC2and4_wram_fix = wram_fix_vrc2and4_183;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_183(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x6800:
			m183.reg[0] = address & 0x3F;
			VRC2and4_wram_fix();
			return;
		case 0x8800:
			m183.reg[1] = value;
			VRC2and4_prg_fix();
			return;
		case 0x9800:
			extcl_cpu_wr_mem_VRC2and4(0x9000, value);
			return;
		case 0xA800:
			m183.reg[2] = value;
			VRC2and4_prg_fix();
			return;
		case 0xA000:
			m183.reg[3] = value;
			VRC2and4_prg_fix();
			return;
		case 0x6000:
		case 0x7000:
		case 0x7800:
		case 0x8000:
		case 0x9000:
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			return;
	}
}
BYTE extcl_save_mapper_183(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m183.reg);
	extcl_save_mapper_VRC2and4(mode, slot, fp);

	return (EXIT_OK);
}

void prg_fix_vrc2and4_183(void) {
	memmap_auto_8k(0x8000, m183.reg[1]);
	memmap_auto_8k(0xA000, m183.reg[2]);
	memmap_auto_8k(0xC000, m183.reg[3]);
	memmap_auto_8k(0xE000, 0xFF);
}
void chr_swap_vrc2and4_183(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}
void wram_fix_vrc2and4_183(void) {
	memmap_prgrom_8k(0x6000, m183.reg[0]);
}
