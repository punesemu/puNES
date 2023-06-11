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

void prg_swap_vrc2and4_384(WORD address, WORD value);
void chr_swap_vrc2and4_384(WORD address, WORD value);

struct _m384 {
	BYTE reg;
} m384;

void map_init_384(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(384);
	EXTCL_SAVE_MAPPER(384);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m384;
	mapper.internal_struct_size[0] = sizeof(m384);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	memset(&m384, 0x00, sizeof(m384));

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, FALSE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_384;
	VRC2and4_chr_swap = chr_swap_vrc2and4_384;
}
void extcl_cpu_wr_mem_384(WORD address, BYTE value) {
	if ((address >= 0x6800) && (address <= 0x6FFF)) {
		if (!(m384.reg & 0x08) && memmap_adr_is_writable(MMCPU(address))) {
			m384.reg = value;
			VRC2and4_prg_fix();
			VRC2and4_chr_fix();
		}
		return;
	}
	extcl_cpu_wr_mem_VRC2and4(address, value);
}
BYTE extcl_save_mapper_384(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m384.reg);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}

void prg_swap_vrc2and4_384(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, ((m384.reg << 4) | (value & 0x0F)));
}
void chr_swap_vrc2and4_384(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, ((m384.reg << 7) | (value & 0x7F)));
}
