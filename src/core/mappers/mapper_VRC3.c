/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

void map_init_VRC3(void) {
	EXTCL_CPU_WR_MEM(VRC3);
	EXTCL_SAVE_MAPPER(VRC3);
	EXTCL_CPU_EVERY_CYCLE(VRC3);
	mapper.internal_struct[0] = (BYTE *) &vrc3;
	mapper.internal_struct_size[0] = sizeof(vrc3);

	info.prg.ram.banks_8k_plus = 1;

	if (info.reset) {
		memset(&vrc3, 0x00, sizeof(vrc3));
		vrc3.mask = 0xFFFF;
	}
}
void extcl_cpu_wr_mem_VRC3(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			vrc3.reload = (vrc3.reload & 0xFFF0) | (value & 0x0F);
			return;
		case 0x9000:
			vrc3.reload = (vrc3.reload & 0xFF0F) | ((value & 0x0F) << 4);
			return;
		case 0xA000:
			vrc3.reload = (vrc3.reload & 0xF0FF) | ((value & 0x0F) << 8);
			return;
		case 0xB000:
			vrc3.reload = (vrc3.reload & 0x0FFF) | ((value & 0x0F) << 12);
			return;
		case 0xC000:
			vrc3.acknowledge = value & 0x01;
			vrc3.enabled = value & 0x02;
			vrc3.mode = value & 0x04;
			vrc3.mask = 0xFFFF;
			if (vrc3.mode) {
				vrc3.mask = 0x00FF;
			}
			if (vrc3.enabled) {
				vrc3.count = vrc3.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xD000:
			vrc3.enabled = vrc3.acknowledge;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF000:
			control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc3.enabled);
	save_slot_ele(mode, slot, vrc3.reload);
	save_slot_ele(mode, slot, vrc3.mode);
	save_slot_ele(mode, slot, vrc3.acknowledge);
	save_slot_ele(mode, slot, vrc3.mask);
	save_slot_ele(mode, slot, vrc3.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC3(void) {
	if (!vrc3.enabled) {
		return;
	}

	if (!(++vrc3.count & vrc3.mask)) {
		vrc3.count = vrc3.reload;
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}
}
