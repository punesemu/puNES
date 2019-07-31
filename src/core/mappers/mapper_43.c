/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#define prg_5000_43()\
	value = 8 << 1;\
	control_bank(info.prg.rom[0].max.banks_4k)\
	prg_5000 = prg_chip_byte_pnt(0, value << 12)
#define prg_6000_swap_43()\
	value = m43.swap ? 0 : 2;\
	control_bank(info.prg.rom[0].max.banks_8k)\
	prg_6000 = prg_chip_byte_pnt(0, value << 13)
#define prg_E000_swap_43()\
	value = m43.swap ? 8 : 9;\
	control_bank(info.prg.rom[0].max.banks_8k)\
	map_prg_rom_8k(1, 3, value)

BYTE *prg_5000, *prg_6000;

void map_init_43(void) {
	EXTCL_CPU_WR_MEM(43);
	EXTCL_CPU_RD_MEM(43);
	EXTCL_SAVE_MAPPER(43);
	EXTCL_CPU_EVERY_CYCLE(43);
	mapper.internal_struct[0] = (BYTE *) &m43;
	mapper.internal_struct_size[0] = sizeof(m43);

	if (info.reset >= HARD) {
		BYTE value = 0;

		memset(&m43, 0x00, sizeof(m43));
		prg_5000_43();
		prg_6000_swap_43();
		map_prg_rom_8k(1, 0, 1);
		map_prg_rom_8k(1, 1, 0);
		map_prg_rom_8k(1, 2, 0);
		prg_E000_swap_43();
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_43(WORD address, BYTE value) {
	switch (address & 0xF1FF) {
		case 0x4022: {
			static const BYTE regs[8] = { 4, 3, 5, 3, 6, 3, 7, 3 };

			value = regs[value & 0x07];
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		}
		case 0x4120: {
			m43.swap = value & 0x01;
			prg_6000_swap_43();
			prg_E000_swap_43();
			map_prg_rom_8k_update();
			return;
		}
		case 0x8122:
		case 0x4122:
			m43.irq.active = value & 0x01;
			m43.irq.count = 0;
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_cpu_rd_mem_43(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x5000) || (address > 0x7FFF)) {
		return (openbus);
	}

	if (address < 0x6000) {
		return (prg_5000[address & 0x0FFF]);
	}

	return (prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_43(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m43.swap);
	save_slot_ele(mode, slot, m43.irq.active);
	save_slot_ele(mode, slot, m43.irq.count);

	if (mode == SAVE_SLOT_READ) {
		BYTE value;

		prg_5000_43();
		prg_6000_swap_43();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_43(void) {
	m43.irq.count++;
	if (m43.irq.active && (m43.irq.count >= 4096)) {
		m43.irq.active = 0;
		irq.high |= EXT_IRQ;
	}
}
