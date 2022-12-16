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

INLINE static void prg_fix_KS7032(void);

struct _ks7032 {
	BYTE ind;
	BYTE reg[8];
	struct _ks7032_irq {
		BYTE enabled;
		WORD count;
		WORD reload;
	} irq;
} ks7032;
struct _ks7032tmp {
	BYTE *prg_6000;
} ks7032tmp;

void map_init_KS7032(void) {
	EXTCL_CPU_WR_MEM(KS7032);
	EXTCL_CPU_RD_MEM(KS7032);
	EXTCL_SAVE_MAPPER(KS7032);
	EXTCL_CPU_EVERY_CYCLE(KS7032);
	mapper.internal_struct[0] = (BYTE *)&ks7032;
	mapper.internal_struct_size[0] = sizeof(ks7032);

	memset(&ks7032, 0x00, sizeof(ks7032));

	prg_fix_KS7032();
}
void extcl_cpu_wr_mem_KS7032(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			ks7032.irq.reload = (ks7032.irq.reload & 0xFFF0) | (value & 0x0F);
			return;
		case 0x9000:
			ks7032.irq.reload = (ks7032.irq.reload & 0xFF0F) | ((value & 0x0F) << 4);
			return;
		case 0xA000:
			ks7032.irq.reload = (ks7032.irq.reload & 0xF0FF) | ((value & 0x0F) << 8);
			return;
		case 0xB000:
			ks7032.irq.reload = (ks7032.irq.reload & 0x0FFF) | ((value & 0x0F) << 12);
			return;
		case 0xC000:
			ks7032.irq.enabled = value & 0x0F;
			if (ks7032.irq.enabled) {
				ks7032.irq.count = ks7032.irq.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xD000:
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE000:
			ks7032.ind = value;
			return;
		case 0xF000:
			ks7032.reg[ks7032.ind & 0x07] = value;
			prg_fix_KS7032();
			return;
	}
}
BYTE extcl_cpu_rd_mem_KS7032(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (ks7032tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7032(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7032.ind);
	save_slot_ele(mode, slot, ks7032.reg);
	save_slot_ele(mode, slot, ks7032.irq.enabled);
	save_slot_ele(mode, slot, ks7032.irq.count);
	save_slot_ele(mode, slot, ks7032.irq.reload);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_KS7032();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_KS7032(void) {
	if (!ks7032.irq.enabled) {
		return;
	}

	if (++ks7032.irq.count == 0xFFFF) {
		ks7032.irq.count = ks7032.irq.reload;
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_KS7032(void) {
	WORD value;

	value = ks7032.reg[4];
	control_bank(info.prg.rom.max.banks_8k)
	ks7032tmp.prg_6000 = prg_pnt(value << 13);

	value = ks7032.reg[1];
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, value);

	value = ks7032.reg[2];
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, value);

	value = ks7032.reg[3];
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, value);

	map_prg_rom_8k_update();
}
