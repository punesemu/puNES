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
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

struct _m237 {
	WORD reg[2];
} m237;
struct _m237tmp {
	BYTE dipswitch;
} m237tmp;

void map_init_237(void) {
	EXTCL_CPU_WR_MEM(237);
	EXTCL_CPU_RD_MEM(237);
	EXTCL_SAVE_MAPPER(237);
	mapper.internal_struct[0] = (BYTE *)&m237;
	mapper.internal_struct_size[0] = sizeof(m237);

	memset(&m237, 0x00, sizeof(m237));

	if (info.reset == RESET) {
		m237tmp.dipswitch = (m237tmp.dipswitch + 1) & 0x03;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m237tmp.dipswitch = 0;
	}

	{
		BYTE value;

		map_prg_rom_8k(2, 0, 0);
		value = 7;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_237(WORD address, BYTE value) {
	if (m237.reg[0] & 0x0002) {
		m237.reg[1] = (m237.reg[1] & ~0x07) | (value & 0x07);
	} else {
		m237.reg[0] = address;
		m237.reg[1] = value;
	}

	{
		BYTE bank[2], outer = ((m237.reg[0] & 0x0004) << 3) | (m237.reg[1] & 0x18);

		switch (m237.reg[1] & 0xC0) {
			default:
			case 0x00:
				bank[0] = outer | (m237.reg[1] & 0x07);
				bank[1] = outer | 0x07;
				break;
			case 0x40:
				bank[0] = outer | (m237.reg[1] & 0x06);
				bank[1] = outer | 0x07;
				break;
			case 0x80:
				bank[0] = bank[1] = outer | (m237.reg[1] & 0x07);
				break;
			case 0xC0:
				bank[0] = outer | (m237.reg[1] & 0x06);
				bank[1] = outer | (m237.reg[1] & 0x06) | 0x01;
				break;
		}

		_control_bank(bank[0], info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank[0]);

		_control_bank(bank[1], info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank[1]);

		map_prg_rom_8k_update();

		if (m237.reg[1] & 0x20) {
			mirroring_H();
		} else {
			mirroring_V();
		}
	}
}
BYTE extcl_cpu_rd_mem_237(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0x8000) {
		return (m237.reg[0] & 0x0001 ? m237tmp.dipswitch : openbus);
	}
	return (openbus);
}
BYTE extcl_save_mapper_237(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m237.reg);

	return (EXIT_OK);
}
