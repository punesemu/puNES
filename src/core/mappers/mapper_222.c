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
#include "cpu.h"
#include "save_slot.h"

void prg_swap_222(WORD address, WORD value);
void chr_swap_222(WORD address, WORD value);

struct _m222 {
	BYTE prescaler;
	BYTE mode;
	BYTE count[2];
	BYTE pending;
} m222;

void map_init_222(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(222);
	EXTCL_CPU_RD_MEM(VRC2and4);
	EXTCL_SAVE_MAPPER(222);
	EXTCL_CPU_EVERY_CYCLE(222);
	mapper.internal_struct[0] = (BYTE *)&m222;
	mapper.internal_struct_size[0] = sizeof(m222);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	memset(&m222, 0x00, sizeof(m222));

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE);
	VRC2and4_prg_swap = prg_swap_222;
	VRC2and4_chr_swap = chr_swap_222;
}
void extcl_cpu_wr_mem_222(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
			if (!(address & 0x0001)) {
				extcl_cpu_wr_mem_VRC2and4(address, value);
				extcl_cpu_wr_mem_VRC2and4(address | 0x0001, value >> 4);
			}
			return;
		case 0xF000:
			switch (address & 0x0003) {
				case 0:
					m222.mode = FALSE;
					break;
				case 1:
					if (!m222.mode) {
						m222.count[0] = value & 0x0F;
						m222.count[1] = value >> 4;
					}
					m222.pending = FALSE;
					break;
				case 2:
					m222.mode = TRUE;
					break;
			}
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			return;
	}
}
BYTE extcl_save_mapper_222(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m222.prescaler);
	save_slot_ele(mode, slot, m222.mode);
	save_slot_ele(mode, slot, m222.count);
	save_slot_ele(mode, slot, m222.pending);
	extcl_save_mapper_VRC2and4(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_222(void) {
	BYTE save = m222.prescaler;

	m222.prescaler = m222.pending ? 0 : m222.prescaler + 1;
	if (m222.mode && !(save & 0x40) && (m222.prescaler & 0x40)) {
		if ((++m222.count[0] == 0x0F) && (++m222.count[1] == 0x0F)) {
			m222.pending = TRUE;
		}
		m222.count[0] &= 0x0F;
		m222.count[1] &= 0x0F;
	}
	if (m222.pending) {
		irq.high |= EXT_IRQ;
	} else {
		irq.high &= ~EXT_IRQ;
	}
}

void prg_swap_222(WORD address, WORD value) {
	prg_swap_VRC2and4(address, (value & 0x1F));
}
void chr_swap_222(WORD address, WORD value) {
	chr_swap_VRC2and4(address, (value & 0xFFF));
}
