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
#include "save_slot.h"

void map_init_166(void) {
	EXTCL_CPU_WR_MEM(166);
	EXTCL_SAVE_MAPPER(166);
	mapper.internal_struct[0] = (BYTE *) &m166;
	mapper.internal_struct_size[0] = sizeof(m166);

	memset(&m166, 0x00, sizeof(m166));

	extcl_cpu_wr_mem_166(0x0000, 0x00);
}
void extcl_cpu_wr_mem_166(WORD address, BYTE value) {
	WORD base, bank;

	m166.reg[(address & 0x6000) >> 13] = value;
	base = (((m166.reg[0] ^ m166.reg[1]) & 0x10) << 1) + ((m166.reg[2] ^ m166.reg[3]) & 0x1F);

	if (m166.reg[1] & 0x08) {
		base &= 0xFFFE;

		bank = base;
		_control_bank(bank, info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = base | 0x0001;
		_control_bank(bank, info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	} else {
		if (m166.reg[1] & 0x04) {
			bank = 0x001F;
			_control_bank(bank, info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, bank);

			bank = base;
			_control_bank(bank, info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
		} else {
			bank = base;
			_control_bank(bank, info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, bank);

			bank = 0x0007;
			_control_bank(bank, info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
		}
	}
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_166(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m166.reg);

	return (EXIT_OK);
}
