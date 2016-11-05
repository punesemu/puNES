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

static void INLINE ks7032_update(void);

BYTE *ks7032_prg_6000;

void map_init_KS7032(void) {
	EXTCL_CPU_WR_MEM(KS7032);
	EXTCL_CPU_RD_MEM(KS7032);
	EXTCL_SAVE_MAPPER(KS7032);
	EXTCL_CPU_EVERY_CYCLE(KS7032);
	mapper.internal_struct[0] = (BYTE *) &ks7032;
	mapper.internal_struct_size[0] = sizeof(ks7032);

	memset(&ks7032, 0x00, sizeof(ks7032));

	ks7032_update();
}
void extcl_cpu_wr_mem_KS7032(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			ks7032.irq.used = 1;
			ks7032.irq.count = (ks7032.irq.count & 0x000F) | (value & 0x0F);
			irq.high &= ~EXT_IRQ;
			return;
		case 0x9000:
			ks7032.irq.used = 1;
			ks7032.irq.count = (ks7032.irq.count & 0x00F0) | (value << 4);
			irq.high &= ~EXT_IRQ;
			return;
		case 0xA000:
			ks7032.irq.used = 1;
			ks7032.irq.count = (ks7032.irq.count & 0x0F00) | (value << 8);
			irq.high &= ~EXT_IRQ;
			return;
		case 0xB000:
			ks7032.irq.used = 1;
			ks7032.irq.count = (ks7032.irq.count & 0xF000) | (value << 12);
			irq.high &= ~EXT_IRQ;
			return;
		case 0xC000:
			if (ks7032.irq.used) {
				ks7032.irq.active = 1;
				irq.high &= ~EXT_IRQ;
			}
			return;
		case 0xE000:
			ks7032.ind = value & 0x07;
			return;
		case 0xF000:
			ks7032.reg[ks7032.ind] = value;
			ks7032_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_KS7032(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (ks7032_prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7032(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7032.ind);
	save_slot_ele(mode, slot, ks7032.reg);
	save_slot_ele(mode, slot, ks7032.irq.used);
	save_slot_ele(mode, slot, ks7032.irq.active);
	save_slot_ele(mode, slot, ks7032.irq.count);
	save_slot_ele(mode, slot, ks7032.irq.reload);

	if (mode == SAVE_SLOT_READ) {
		ks7032_update();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_KS7032(void) {
	if (!ks7032.irq.active) {
		return;
	}

	if (++ks7032.irq.count == 0xFFFF) {
		ks7032.irq.active = ks7032.irq.count = 0;
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}
}
static void INLINE ks7032_update(void) {
	WORD value;

	value = ks7032.reg[4];
	control_bank(info.prg.rom[0].max.banks_8k)
	ks7032_prg_6000 = prg_chip_byte_pnt(0, value << 13);

    value = ks7032.reg[1];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);

    value = ks7032.reg[2];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);

    value = ks7032.reg[3];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 2, value);

	map_prg_rom_8k_update();
}
