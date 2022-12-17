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
#include "cpu.h"
#include "save_slot.h"

struct btl900218 {
	BYTE irq_enabled;
	WORD irq_counter;
} btl900218;

void map_init_BTL900218(void) {
	map_init_VRC2(VRC2B, 0x1F);

	EXTCL_CPU_WR_MEM(BTL900218);
	EXTCL_SAVE_MAPPER(BTL900218);
	EXTCL_CPU_EVERY_CYCLE(BTL900218);
	mapper.internal_struct[1] = (BYTE *)&btl900218;
	mapper.internal_struct_size[1] = sizeof(btl900218);

	memset(&btl900218, 0x00, sizeof(btl900218));
}
void extcl_cpu_wr_mem_BTL900218(WORD address, BYTE value) {
	switch (address & 0xF00C) {
		case 0xF008:
			btl900218.irq_enabled = TRUE;
			return;
		case 0xF00C :
			btl900218.irq_enabled = FALSE;
			btl900218.irq_counter = 0;
			irq.high &= ~EXT_IRQ;
			return;
	}
	extcl_cpu_wr_mem_VRC2(address, value);
}
BYTE extcl_save_mapper_BTL900218(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, btl900218.irq_enabled);
	save_slot_ele(mode, slot, btl900218.irq_counter);
	extcl_save_mapper_VRC2(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_BTL900218(void) {
	if (btl900218.irq_enabled && (++btl900218.irq_counter & 0x400)) {
		irq.high |= EXT_IRQ;
	}
}
