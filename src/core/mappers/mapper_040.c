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

struct _m040 {
	BYTE enabled;
	WORD count;
	BYTE delay;
} m040;
struct _m040tmp {
	BYTE *prg_6000;
} m040tmp;

void map_init_040(void) {
	EXTCL_CPU_WR_MEM(040);
	EXTCL_CPU_RD_MEM(040);
	EXTCL_SAVE_MAPPER(040);
	EXTCL_CPU_EVERY_CYCLE(040);
	mapper.internal_struct[0] = (BYTE *)&m040;
	mapper.internal_struct_size[0] = sizeof(m040);

	if (info.reset >= HARD) {
		memset(&m040, 0x00, sizeof(40));

		mapper.rom_map_to[2] = 0;
	}

	m040tmp.prg_6000 = prg_pnt(6 << 13);

	mapper.rom_map_to[0] = 4;
	mapper.rom_map_to[1] = 5;
	mapper.rom_map_to[3] = 7;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_040(WORD address, BYTE value) {
	if (address < 0x8000) {
		return;
	}

	switch (address & 0xE000) {
		case 0x8000:
			m040.enabled = FALSE;
			m040.count = 0;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xA000:
			m040.enabled = TRUE;
			return;
		case 0xE000:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_040(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m040tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_040(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m040.enabled);
	save_slot_ele(mode, slot, m040.count);
	save_slot_ele(mode, slot, m040.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_040(void) {
	if (m040.delay && !(--m040.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (m040.enabled && (++m040.count == 0x1000)) {
		m040.delay = 1;
	}
}
