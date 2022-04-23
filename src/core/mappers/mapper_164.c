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
#include "ppu.h"
#include "save_slot.h"

struct _m164 {
	BYTE prg;
} m164;

void map_init_164(void) {
	EXTCL_CPU_WR_MEM(164);
	EXTCL_CPU_RD_MEM(164);
	EXTCL_SAVE_MAPPER(164);
	mapper.internal_struct[0] = (BYTE *)&m164;
	mapper.internal_struct_size[0] = sizeof(m164);

	memset(&m164, 0x00, sizeof(m164));
	m164.prg = 0x0F;

	{
		BYTE value = m164.prg;

		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_164(WORD address, BYTE value) {
	switch (address & 0x7300) {
		case 0x5000:
			m164.prg = (m164.prg & 0xF0) | (value & 0x0F);
			value = m164.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x5100:
			m164.prg = (m164.prg & 0x0F) | (value << 4);
			value = m164.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		/*
		case 0x5200:
			return;
		case 0x5300:
			value = m164.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		*/
	}
}
BYTE extcl_cpu_rd_mem_164(WORD address, BYTE openbus, BYTE before) {
	if ((address > 0x4FFF) && (address < 0x6000)) {
		return (before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_164(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m164.prg);

	return (EXIT_OK);
}
