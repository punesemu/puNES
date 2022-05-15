/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "save_slot.h"

struct th21311 {
	BYTE irq_enabled;
	WORD irq_h_counter;
	WORD irq_l_counter;
} th21311;

void map_init_TH21311(void) {
	map_init_VRC2(VRC2B, 0x1F);

	EXTCL_CPU_WR_MEM(TH21311);
	EXTCL_SAVE_MAPPER(TH21311);
	EXTCL_CPU_EVERY_CYCLE(TH21311);
	mapper.internal_struct[1] = (BYTE *)&th21311;
	mapper.internal_struct_size[1] = sizeof(th21311);

	memset(&th21311, 0x00, sizeof(th21311));
}
void extcl_cpu_wr_mem_TH21311(WORD address, BYTE value) {
	switch (address & 0xF003) {
		case 0xF000:
			th21311.irq_enabled = FALSE;
			th21311.irq_l_counter = 0;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF001 :
			th21311.irq_enabled = TRUE;
			return;
		case 0xF003:
			th21311.irq_h_counter = value >> 4;
			return;
	}
	extcl_cpu_wr_mem_VRC2(address, value);
}
BYTE extcl_save_mapper_TH21311(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, th21311.irq_enabled);
	save_slot_ele(mode, slot, th21311.irq_h_counter);
	save_slot_ele(mode, slot, th21311.irq_l_counter);
	extcl_save_mapper_VRC2(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_TH21311(void) {
	if (th21311.irq_enabled) {
		th21311.irq_l_counter = (th21311.irq_l_counter + 1) & 0x0FFF;
		if (th21311.irq_l_counter == 0x800) {
			th21311.irq_h_counter--;
		}
		if (!th21311.irq_h_counter && (th21311.irq_l_counter < 0x800)) {
			irq.high |= EXT_IRQ;
		}
	}
}
