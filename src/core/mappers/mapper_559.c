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

void prg_swap_vrc2and4_559(WORD address, WORD value);
void chr_swap_vrc2and4_559(WORD address, WORD value);
void mirroring_fix_vrc2and4_559(void);
void misc_03_vrc2and4_559(WORD address, BYTE value);

struct _m559 {
	BYTE prg;
	BYTE mir[4];
} m559;

void map_init_559(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(559);
	EXTCL_SAVE_MAPPER(559);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m559;
	mapper.internal_struct_size[0] = sizeof(m559);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	memset(&m559, 0x00, sizeof(m559));

	init_VRC2and4(VRC24_VRC4, 0x400, 0x800, TRUE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_559;
	VRC2and4_chr_swap = chr_swap_vrc2and4_559;
	VRC2and4_mirroring_fix = mirroring_fix_vrc2and4_559;
	VRC2and4_misc_03 = misc_03_vrc2and4_559;

	m559.prg = 0xFE;
	m559.mir[0] = 0xE0;
	m559.mir[1] = 0xE0;
	m559.mir[2] = 0xE1;
	m559.mir[3] = 0xE1;
}
void extcl_cpu_wr_mem_559(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (address & 0x0400) {
				value >>= 4;
			}
			extcl_cpu_wr_mem_VRC2and4(address, value);
			break;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			break;
	}
}
BYTE extcl_save_mapper_559(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m559.prg);
	save_slot_ele(mode, slot, m559.mir);
	extcl_save_mapper_VRC2and4(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		VRC2and4_mirroring_fix();
	}

	return (EXIT_OK);
}

void prg_swap_vrc2and4_559(WORD address, WORD value) {
	WORD mask = 0x1F;

	if (((address >> 13) & 0x03) == 2) {
		value = m559.prg;
		mask = 0xFF;
	}
	prg_swap_VRC2and4_base(address, (value & mask));
}
void chr_swap_vrc2and4_559(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}
void mirroring_fix_vrc2and4_559(void) {
	WORD bank = 0;

	bank = m559.mir[0];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	ntbl.bank_1k[0] = chr_pnt(bank << 10);

	bank = m559.mir[1];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	ntbl.bank_1k[1] = chr_pnt(bank << 10);

	bank = m559.mir[2];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	ntbl.bank_1k[2] = chr_pnt(bank << 10);

	bank = m559.mir[3];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	ntbl.bank_1k[3] = chr_pnt(bank << 10);
}
void misc_03_vrc2and4_559(WORD address, BYTE value) {
	if (address & 0x0004) {
		m559.mir[address & 0x03] = value;
		VRC2and4_mirroring_fix();
	} else {
		m559.prg = value;
		VRC2and4_prg_fix();
	}
}
