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
#include "cpu.h"
#include "save_slot.h"

void prg_swap_vrc2and4_308(WORD address, WORD value);
void chr_swap_vrc2and4_308(WORD address, WORD value);

struct _m308 {
	struct _m308_irq {
		BYTE enabled;
		WORD h_counter;
		WORD l_counter;
	} irq;
} m308;

void map_init_308(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(308);
	EXTCL_SAVE_MAPPER(308);
	EXTCL_CPU_EVERY_CYCLE(308);
	mapper.internal_struct[0] = (BYTE *)&m308;
	mapper.internal_struct_size[0] = sizeof(m308);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	memset(&m308, 0x00, sizeof(m308));

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE);
	VRC2and4_prg_swap = prg_swap_vrc2and4_308;
	VRC2and4_chr_swap = chr_swap_vrc2and4_308;
}
void extcl_cpu_wr_mem_308(WORD address, BYTE value) {
	switch (address & 0xF003) {
		case 0xF000:
			m308.irq.enabled = FALSE;
			m308.irq.l_counter = 0;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF001 :
			m308.irq.enabled = TRUE;
			return;
		case 0xF003:
			m308.irq.h_counter = value >> 4;
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			return;
	}
}
BYTE extcl_save_mapper_308(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m308.irq.enabled);
	save_slot_ele(mode, slot, m308.irq.h_counter);
	save_slot_ele(mode, slot, m308.irq.l_counter);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
void extcl_cpu_every_cycle_308(void) {
	if (m308.irq.enabled) {
		m308.irq.l_counter = (m308.irq.l_counter + 1) & 0x0FFF;
		if (m308.irq.l_counter == 0x800) {
			m308.irq.h_counter--;
		}
		if (!m308.irq.h_counter && (m308.irq.l_counter < 0x800)) {
			irq.high |= EXT_IRQ;
		}
	}
}

void prg_swap_vrc2and4_308(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_308(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0xFF));
}
