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
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_368(void);

struct _m368 {
	BYTE reg[2];
	struct _m368_irq {
		BYTE enable;
		WORD counter;
	} irq;
} m368;
struct _m368tmp {
	BYTE *prg_6000;
} m368tmp;

void map_init_368(void) {
	EXTCL_AFTER_MAPPER_INIT(368);
	EXTCL_CPU_WR_MEM(368);
	EXTCL_CPU_RD_MEM(368);
	EXTCL_SAVE_MAPPER(368);
	EXTCL_CPU_EVERY_CYCLE(368);
	mapper.internal_struct[0] = (BYTE *)&m368;
	mapper.internal_struct_size[0] = sizeof(m368);

	memset(&m368, 0x00, sizeof(m368));

	info.mapper.extend_wr = TRUE;
	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
}
void extcl_after_mapper_init_368(void) {
	prg_fix_368();
}
void extcl_cpu_wr_mem_368(WORD address, BYTE value) {
		switch (address & 0xF1FF)
			case 0x4022: {
				m368.reg[0] = value;
				prg_fix_368();
				break;
			case 0x4122:
				m368.reg[1] = value;
				m368.irq.enable = value & 0x01;
				if (!m368.irq.enable) {
					m368.irq.counter = 0;
					irq.high &= ~EXT_IRQ;
				}
				break;
			default:
				break;
		}
}
BYTE extcl_cpu_rd_mem_368(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x4000:
			if ((address & 0x01FF) == 0x0122) {
				return (0x8A | (m368.reg[1] & 0x35));
			}
			break;
		case 0x6000:
		case 0x7000:
			return (m368tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_368(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m368.reg);
	save_slot_ele(mode, slot, m368.irq.enable);
	save_slot_ele(mode, slot, m368.irq.counter);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_368();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_368(void) {
	if (m368.irq.enable) {
		m368.irq.counter = (m368.irq.counter + 1) & 0x0FFF;
		if (!m368.irq.counter) {
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_368(void) {
	WORD bank;

	bank = 0x02;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m368tmp.prg_6000 = prg_pnt(bank << 13);

	bank = 0x01;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = 0x00;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	// Value  Bank#
	// ------------
	// 0      4
	// 1      3
	// 2      5
	// 3      3
	// 4      6
	// 5      3
	// 6      7
	// 7      3
	bank = m368.reg[0] & 0x01 ? 3 : 4 | ((m368.reg[0] & 0x06) >> 1);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0x08;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
