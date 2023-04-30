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

#define prg_5000_043()\
	value = 8 << 1;\
	control_bank(info.prg.rom.max.banks_4k)\
	m043tmp.prg_5000 = prg_pnt(value << 12)
#define prg_6000_swap_043()\
	value = m043.swap ? 0 : 2;\
	control_bank(info.prg.rom.max.banks_8k)\
	m043tmp.prg_6000 = prg_pnt(value << 13)
#define prg_E000_swap_043()\
	value = m043.swap ? 8 : 9;\
	control_bank(info.prg.rom.max.banks_8k)\
	map_prg_rom_8k(1, 3, value)

struct _m043 {
	BYTE swap;
	struct _m043_irq {
		BYTE active;
		WORD count;
	} irq;
} m043;
struct _m043tmp {
	BYTE *prg_5000;
	BYTE *prg_6000;
} m043tmp;

void map_init_043(void) {
	EXTCL_CPU_WR_MEM(043);
	EXTCL_CPU_RD_MEM(043);
	EXTCL_SAVE_MAPPER(043);
	EXTCL_CPU_EVERY_CYCLE(043);
	mapper.internal_struct[0] = (BYTE *)&m043;
	mapper.internal_struct_size[0] = sizeof(m043);

	if (info.reset >= HARD) {
		BYTE value = 0;

		memset(&m043, 0x00, sizeof(m043));
		prg_5000_043();
		prg_6000_swap_043();
		map_prg_rom_8k(1, 0, 1);
		map_prg_rom_8k(1, 1, 0);
		map_prg_rom_8k(1, 2, 0);
		prg_E000_swap_043();
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_043(WORD address, BYTE value) {
	switch (address & 0xF1FF) {
		case 0x4022: {
			static const BYTE regs[8] = { 4, 3, 5, 3, 6, 3, 7, 3 };

			value = regs[value & 0x07];
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		}
		case 0x4120: {
			m043.swap = value & 0x01;
			prg_6000_swap_043();
			prg_E000_swap_043();
			map_prg_rom_8k_update();
			return;
		}
		case 0x8122:
		case 0x4122:
			m043.irq.active = value & 0x01;
			m043.irq.count = 0;
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_cpu_rd_mem_043(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x5000) || (address > 0x7FFF)) {
		return (openbus);
	}

	if (address < 0x6000) {
		return (m043tmp.prg_5000[address & 0x0FFF]);
	}

	return (m043tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_043(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m043.swap);
	save_slot_ele(mode, slot, m043.irq.active);
	save_slot_ele(mode, slot, m043.irq.count);

	if (mode == SAVE_SLOT_READ) {
		BYTE value;

		prg_5000_043();
		prg_6000_swap_043();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_043(void) {
	m043.irq.count++;
	if (m043.irq.active && (m043.irq.count >= 4096)) {
		m043.irq.active = 0;
		irq.high |= EXT_IRQ;
	}
}
