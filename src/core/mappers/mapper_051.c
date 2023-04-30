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
#include "save_slot.h"

struct _m051 {
	BYTE mode;
	WORD bank;
	BYTE prg_6000;
} m051;
struct _m051tmp {
	BYTE *prg_6000;
} m051tmp;

void map_init_051(void) {
	EXTCL_CPU_WR_MEM(051);
	EXTCL_CPU_RD_MEM(051);
	EXTCL_SAVE_MAPPER(051);
	mapper.internal_struct[0] = (BYTE *)&m051;
	mapper.internal_struct_size[0] = sizeof(m051);

	if (info.reset >= HARD) {
		memset(&m051, 0x00, sizeof(m051));

		extcl_cpu_wr_mem_051(0x6000, 0x02);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_051(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	if (address >= 0xE000) {
		m051.bank = value & 0x0F;
	} else if (address >= 0xC000) {
		m051.bank = value & 0x0F;
		m051.mode = ((value >> 3) & 0x02) | (m051.mode & 0x01);
	} else if (address >= 0x8000) {
		m051.bank = value & 0x0F;
	} else {
		m051.mode = ((value >> 3) & 0x02) | ((value >> 1) & 0x01);
	}

	if (m051.mode & 0x01) {
		m051.prg_6000 = 0x23;

		value = m051.bank;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		m051.prg_6000 = 0x2F;

		value = (m051.bank << 1) | (m051.mode >> 1);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);

		value = (m051.bank << 1) | 0x07;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	m051.prg_6000 = m051.prg_6000 | (m051.bank << 2);
	_control_bank(m051.prg_6000, info.prg.rom.max.banks_8k)
	m051tmp.prg_6000 = prg_pnt(m051.prg_6000 << 13);

	if (m051.mode == 0x03) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
BYTE extcl_cpu_rd_mem_051(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m051tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_051(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m051.bank);
	save_slot_ele(mode, slot, m051.mode);
	save_slot_ele(mode, slot, m051.prg_6000);

	if (mode == SAVE_SLOT_READ) {
		m051tmp.prg_6000 = prg_pnt(m051.prg_6000 << 13);
	}

	return (EXIT_OK);
}
