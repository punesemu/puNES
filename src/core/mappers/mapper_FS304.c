/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include "mappers.h"
#include "info.h"
#include "save_slot.h"

void map_init_FS304(void) {
	EXTCL_CPU_WR_MEM(FS304);
	EXTCL_SAVE_MAPPER(FS304);
	mapper.internal_struct[0] = (BYTE *) &fs304;
	mapper.internal_struct_size[0] = sizeof(fs304);

	fs304.reg[0] = 3;
	fs304.reg[1] = 0;
	fs304.reg[2] = 0;
	fs304.reg[3] = 7;

	extcl_cpu_wr_mem_FS304(0x5001, 0);

	info.mapper.extend_wr = TRUE;

	info.prg.ram.banks_8k_plus = 1;
	info.prg.ram.bat.banks = 1;
}
void extcl_cpu_wr_mem_FS304(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		fs304.reg[(address >> 8) & 0x03] = value;
		switch (fs304.reg[3] & 0x07) {
			case 0:
			case 2:
				value = (fs304.reg[0] & 0x0C) | (fs304.reg[1] & 0x02) |
						((fs304.reg[2] & 0x0F) << 4);
				break;
			case 1:
			case 3:
				value = (fs304.reg[0] & 0x0C) | ((fs304.reg[2] & 0x0F) << 4);
				break;
			case 4:
			case 6:
				value = (fs304.reg[0] & 0x0E) | ((fs304.reg[1] >> 1) & 0x01) |
						((fs304.reg[2] & 0x0F) << 4);
				break;
			case 5:
			case 7:
				value = (fs304.reg[0] & 0x0F) | ((fs304.reg[2] & 0x0F) << 4);
				break;
		}
		control_bank(info.prg.rom[0].max.banks_32k);
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		return;
	}
}
BYTE extcl_save_mapper_FS304(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, fs304.reg);

	return (EXIT_OK);
}
