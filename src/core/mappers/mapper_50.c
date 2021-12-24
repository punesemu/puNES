/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

struct _m50 {
	BYTE enabled;
	WORD count;
	BYTE delay;
} m50;
struct _m50tmp {
	BYTE *prg_6000;
} m50tmp;

void map_init_50(void) {
	EXTCL_CPU_WR_MEM(50);
	EXTCL_CPU_RD_MEM(50);
	EXTCL_SAVE_MAPPER(50);
	EXTCL_CPU_EVERY_CYCLE(50);
	mapper.internal_struct[0] = (BYTE *)&m50;
	mapper.internal_struct_size[0] = sizeof(m50);

	if (info.reset >= HARD) {
		memset(&m50, 0x00, sizeof(m50));

		mapper.rom_map_to[2] = 0;
	}

	m50tmp.prg_6000 = prg_chip_byte_pnt(0, info.prg.rom[0].max.banks_8k << 13);

	mapper.rom_map_to[0] = 8;
	mapper.rom_map_to[1] = 9;
	mapper.rom_map_to[3] = 11;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_50(WORD address, BYTE value) {
	if ((address <= 0x5FFF) && ((address & 0x0060) == 0x0020)) {
		if (address & 0x0100) {
			if (!(m50.enabled = value & 0x01)) {
				m50.count = 0;
				irq.high &= ~EXT_IRQ;
			}
			return;
		}

		value = (value & 0x08) | ((value << 2) & 0x04) | ((value >> 1) & 0x03);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);
		map_prg_rom_8k_update();
		return;
	}
}
BYTE extcl_cpu_rd_mem_50(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m50tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_50(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m50.enabled);
	save_slot_ele(mode, slot, m50.count);
	save_slot_ele(mode, slot, m50.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_50(void) {
	if (m50.delay && !(--m50.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (m50.enabled && (++m50.count == 0x1000)) {
		m50.delay = 1;
	}
}
