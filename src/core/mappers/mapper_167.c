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
#include "save_slot.h"

void map_init_167(void) {
	EXTCL_CPU_WR_MEM(167);
	EXTCL_SAVE_MAPPER(167);
	mapper.internal_struct[0] = (BYTE *) &m167;
	mapper.internal_struct_size[0] = sizeof(m167);

	memset(&m167, 0x00, sizeof(m167));

	extcl_cpu_wr_mem_167(0x0000, 0x00);
}
void extcl_cpu_wr_mem_167(WORD address, BYTE value) {
	WORD base, bank;

	m167.reg[(address & 0x6000) >> 13] = value;
	base = (((m167.reg[0] ^ m167.reg[1]) & 0x10) << 1) + ((m167.reg[2] ^ m167.reg[3]) & 0x1F);

	if (m167.reg[1] & 0x08) {
		base &= 0xFFFE;

		bank = base | 0x0001;
		_control_bank(bank, info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = base;
		_control_bank(bank, info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	} else {
		if (m167.reg[1] & 0x04) {
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

			bank = 0x0020;
			_control_bank(bank, info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
		}
	}
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_167(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m167.reg);

	return (EXIT_OK);
}
