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
#include "save_slot.h"

struct _m51 {
	BYTE mode;
	WORD bank;
	BYTE prg_6000;
} m51;
struct _m51tmp {
	BYTE *prg_6000;
} m51tmp;

void map_init_51(void) {
	EXTCL_CPU_WR_MEM(51);
	EXTCL_CPU_RD_MEM(51);
	EXTCL_SAVE_MAPPER(51);
	mapper.internal_struct[0] = (BYTE *) &m51;
	mapper.internal_struct_size[0] = sizeof(m51);

	if (info.reset >= HARD) {
		memset(&m51, 0x00, sizeof(m51));

		extcl_cpu_wr_mem_51(0x6000, 0x02);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_51(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	if (address >= 0xE000) {
		m51.bank = value & 0x0F;
	} else if (address >= 0xC000) {
		m51.bank = value & 0x0F;
		m51.mode = ((value >> 3) & 0x02) | (m51.mode & 0x01);
	} else if (address >= 0x8000) {
		m51.bank = value & 0x0F;
	} else {
		m51.mode = ((value >> 3) & 0x02) | ((value >> 1) & 0x01);
	}

	if (m51.mode & 0x01) {
		m51.prg_6000 = 0x23;

		value = m51.bank;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		m51.prg_6000 = 0x2F;

		value = (m51.bank << 1) | (m51.mode >> 1);
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);

		value = (m51.bank << 1) | 0x07;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	m51.prg_6000 = m51.prg_6000 | (m51.bank << 2);
	_control_bank(m51.prg_6000, info.prg.rom[0].max.banks_8k)
	m51tmp.prg_6000 = prg_chip_byte_pnt(0, m51.prg_6000 << 13);

	if (m51.mode == 0x03) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
BYTE extcl_cpu_rd_mem_51(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m51tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_51(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m51.bank);
	save_slot_ele(mode, slot, m51.mode);
	save_slot_ele(mode, slot, m51.prg_6000);

	if (mode == SAVE_SLOT_READ) {
		m51tmp.prg_6000 = prg_chip_byte_pnt(0, m51.prg_6000 << 13);
	}

	return (EXIT_OK);
}
