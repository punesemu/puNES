/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

void map_init_120(void) {
	EXTCL_CPU_WR_MEM(120);
	EXTCL_CPU_RD_MEM(120);
	EXTCL_SAVE_MAPPER(120);
	mapper.internal_struct[0] = (BYTE *) &m120;
	mapper.internal_struct_size[0] = sizeof(m120);

	info.mapper.extend_wr = TRUE;
	cpu.prg_ram_wr_active = FALSE;
	cpu.prg_ram_rd_active = FALSE;

	if (info.reset >= HARD) {
		memset(&m120, 0x00, sizeof(m120));
		m120.prg_ram_rd = prg_chip(0);
		map_prg_rom_8k(4, 0, 2);
	}
}
void extcl_cpu_wr_mem_120(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if ((address & 0xE3C0) == 0x41C0) {
		control_bank_with_AND(0x07, info.prg.rom[0].max.banks_8k)
		m120.prg_ram_rd = prg_chip_byte_pnt(0, value << 13);
	}
}
BYTE extcl_cpu_rd_mem_120(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m120.prg_ram_rd[address & 0x1FFF]);
}
BYTE extcl_save_mapper_120(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_pos(mode, slot, prg_chip(0), m120.prg_ram_rd);

	return (EXIT_OK);
}
