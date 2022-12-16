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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_357(void);
INLINE static void mirroring_fix_357(void);

static BYTE dipswitch_357[4] = { 0x00, 0x08, 0x10, 0x18 };
static BYTE banks_357[2][8] = {
	{ 4, 3, 5, 3, 6, 3, 7, 3 },
	{ 1, 1, 5, 1, 4, 1, 5, 1 }
};
struct _m357 {
	BYTE reg[3];
	struct _m357_dipswitch {
		BYTE actual;
		BYTE index;
	} dipswitch;
	struct _m357_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m357;
struct _m357tmp {
	BYTE *prg_6000;
} m357tmp;

void map_init_357(void) {
	EXTCL_AFTER_MAPPER_INIT(357);
	EXTCL_CPU_WR_MEM(357);
	EXTCL_CPU_RD_MEM(357);
	EXTCL_SAVE_MAPPER(357);
	EXTCL_CPU_EVERY_CYCLE(357);
	mapper.internal_struct[0] = (BYTE *)&m357;
	mapper.internal_struct_size[0] = sizeof(m357);

	m357.reg[0] = 0;
	m357.reg[1] = 0;
	m357.reg[2] = 0;
	m357.irq.enable = 0;
	m357.irq.counter = 0;

	if (info.reset == RESET) {
		m357.dipswitch.index = (m357.dipswitch.index + 1) & 0x03;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m357.dipswitch.index = 0;
	}
	m357.dipswitch.actual = dipswitch_357[m357.dipswitch.index];

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_357(void) {
	prg_fix_357();
	mirroring_fix_357();
}
void extcl_cpu_wr_mem_357(WORD address, BYTE value) {
	if (address >= 0x4000) {
		if (address & 0x8000) {
			m357.reg[0] = value & 0x07;
			prg_fix_357();
		}
		if ((address & 0x71FF) == 0x4022) {
			m357.reg[1] = value & 0x07;
			prg_fix_357();
		}
		if ((address & 0x71FF) == 0x4120) {
			m357.reg[2] = value & 0x01;
			prg_fix_357();
		}
		if ((address & 0xF1FF) == 0x4122) {
			m357.irq.enable = value & 0x01;
			m357.irq.counter = 0;
			irq.high &= ~EXT_IRQ;
		}
	}
}
BYTE extcl_cpu_rd_mem_357(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (m357.dipswitch.actual == 0 ? m357tmp.prg_6000[address & 0x1FFF] : openbus);
	}
	return (openbus);
}
BYTE extcl_save_mapper_357(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m357.reg);
	save_slot_ele(mode, slot, m357.dipswitch.actual);
	save_slot_ele(mode, slot, m357.dipswitch.index);
	save_slot_ele(mode, slot, m357.irq.enable);
	save_slot_ele(mode, slot, m357.irq.counter);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_357();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_357(void) {
	if (m357.irq.enable) {
		m357.irq.counter++;
		if (m357.irq.counter == 0x1000) {
			m357.irq.counter = 0;
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_357(void) {
	WORD value;

	if (m357.dipswitch.actual == 0) {
		value = m357.reg[2] ? 0 : 2;
		control_bank(info.prg.rom.max.banks_8k)
		m357tmp.prg_6000 = prg_pnt(value << 13);

		value = m357.reg[2] ? 0 : 1;
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = 0;
		map_prg_rom_8k(1, 1, value);

		value = banks_357[m357.reg[2]][m357.reg[1]];
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = m357.reg[2] ? 8 : 10;
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	} else {
		value = m357.dipswitch.actual | m357.reg[0];
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);

		value = m357.dipswitch.actual | 0x07;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_357(void) {
	if (m357.dipswitch.actual == 0x18) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
