/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

BYTE *prg_6000;

void map_init_40(void) {
	EXTCL_CPU_WR_MEM(40);
	EXTCL_CPU_RD_MEM(40);
	EXTCL_SAVE_MAPPER(40);
	EXTCL_CPU_EVERY_CYCLE(40);
	mapper.internal_struct[0] = (BYTE *) &m40;
	mapper.internal_struct_size[0] = sizeof(m40);

	if (info.reset >= HARD) {
		memset(&m40, 0x00, sizeof(40));

		mapper.rom_map_to[2] = 0;
	}

	prg_6000 = prg_chip_byte_pnt(0, 6 << 13);

	mapper.rom_map_to[0] = 4;
	mapper.rom_map_to[1] = 5;
	mapper.rom_map_to[3] = 7;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_40(WORD address, BYTE value) {
	if (address < 0x8000) {
		return;
	}

	switch (address & 0xE000) {
		case 0x8000:
			m40.enabled = FALSE;
			m40.count = 0;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xA000:
			m40.enabled = TRUE;
			return;
		case 0xE000:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_40(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_40(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m40.enabled);
	save_slot_ele(mode, slot, m40.count);
	save_slot_ele(mode, slot, m40.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_40(void) {
	if (m40.delay && !(--m40.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (m40.enabled && (++m40.count == 0x1000)) {
		m40.delay = 1;
	}
}
