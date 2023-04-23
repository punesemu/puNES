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

INLINE static void prg_fix_311(void);

struct _m311 {
	BYTE reg;
	struct _m311_irq {
		BYTE enabled;
		uint32_t count;
	} irq;
} m311;
struct _m311tmp {
	BYTE *prg_5000;
	BYTE *prg_6000;
} m311tmp;

void map_init_311(void) {
	EXTCL_AFTER_MAPPER_INIT(311);
	EXTCL_CPU_WR_MEM(311);
	EXTCL_CPU_RD_MEM(311);
	EXTCL_SAVE_MAPPER(311);
	EXTCL_CPU_EVERY_CYCLE(311);
	mapper.internal_struct[0] = (BYTE *)&m311;
	mapper.internal_struct_size[0] = sizeof(m311);

	memset(&m311, 0x00, sizeof(m311));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_311(void) {
	prg_fix_311();
}
void extcl_cpu_wr_mem_311(WORD address, BYTE value) {
	switch (address) {
		case 0x4022:
			m311.reg = value & 0x01;
			prg_fix_311();
			return;
		case 0x4122:
			m311.irq.enabled = value & 0x01;
			m311.irq.count = 0;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_311(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x4000:
			return ((address >= 0x4042) && (address <= 0x4055) ? 0xFF : openbus);
		case 0x5000:
			return (m311tmp.prg_5000[address & 0x0FFF]);
		case 0x6000:
		case 0x7000:
			return (m311tmp.prg_6000[address & 0x1FFF]);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_311(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m311.reg);
	save_slot_ele(mode, slot, m311.irq.enabled);
	save_slot_ele(mode, slot, m311.irq.count);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_311();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_311(void) {
	if (m311.irq.enabled) {
		if (m311.irq.count < 4096) {
			m311.irq.count++;
		} else {
			m311.irq.count++;
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_311(void) {
	WORD bank = 17;

	_control_bank(bank, info.prg.rom.max.banks_4k)
	m311tmp.prg_5000 = prg_pnt(bank << 12);

	bank = 9;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m311tmp.prg_6000 = prg_pnt(bank << 13);

	bank = m311.reg;
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);

	map_prg_rom_8k_update();
}
