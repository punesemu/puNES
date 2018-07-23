/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

void map_init_KS7017(void) {
	EXTCL_CPU_WR_MEM(KS7017);
	EXTCL_CPU_RD_MEM(KS7017);
	EXTCL_SAVE_MAPPER(KS7017);
	EXTCL_CPU_EVERY_CYCLE(KS7017);
	mapper.internal_struct[0] = (BYTE *) &ks7017;
	mapper.internal_struct_size[0] = sizeof(ks7017);

	memset(&ks7017, 0x00, sizeof(ks7017));

	{
		BYTE value;

		value = 2;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}

	info.prg.ram.banks_8k_plus = 1;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_KS7017(WORD address, BYTE value) {
	if (address > 0x5FFF) {
		return;
	}

	if ((address & 0xFF00) == 0x4A00) {
		ks7017.reg =  ((address >> 4) & 0x04) | ((address & 0x0C) >> 2);
	} else if ((address & 0xFF00) == 0x5100) {
		value = ks7017.reg;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k_update();
	} else if (address == 0x4020) {
		irq.high &= ~EXT_IRQ;
		ks7017.irq.count = (ks7017.irq.count & 0xFF00) | value;
	} else if (address == 0x4021) {
		irq.high &= ~EXT_IRQ;
		ks7017.irq.count = (value << 8) | (ks7017.irq.count & 0x00FF);
		ks7017.irq.active = 1;
	} else if (address == 0x4025) {
		if ((value >> 3) & 0x01) {
			mirroring_H();
		} else {
			mirroring_V();
		}
	}
}
BYTE extcl_cpu_rd_mem_KS7017(WORD address, BYTE openbus, BYTE before) {
	if (address == 0x4030) {
		openbus = (irq.high & EXT_IRQ) ? 1 : 0;
		irq.high &= ~EXT_IRQ;
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7017(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7017.reg);
	save_slot_ele(mode, slot, ks7017.irq.active);
	save_slot_ele(mode, slot, ks7017.irq.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_KS7017(void) {
	if (!ks7017.irq.active) {
		return;
	}

	if (--ks7017.irq.count == 0) {
		ks7017.irq.active = 0;
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}
}
