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
#include "cpu.h"
#include "save_slot.h"

void map_init_36(void) {
	EXTCL_CPU_WR_MEM(36);
	EXTCL_CPU_RD_MEM(36);
	EXTCL_SAVE_MAPPER(36);
	mapper.internal_struct[0] = (BYTE *) &m36;
    mapper.internal_struct_size[0] = sizeof(m36);

	memset(&m36, 0x00, sizeof(m36));

	map_prg_rom_8k_reset();
	map_chr_bank_1k_reset();

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_36(WORD address, BYTE value) {
	if ((address & 0xE200) == 0x4200) {
		DBWORD bank;

		control_bank(info.chr.rom[0].max.banks_8k)
		bank = value << 13;
		chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
		chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
		chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
		chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
		chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
	}

	switch (address & 0xE103) {
		case 0x4100:
			m36.regs[0] = value;

			if (m36.regs[3] & 0x10) {
				m36.regs[4]++;
			} else {
				m36.regs[4] = m36.regs[2];
			}
			break;
		case 0x4101:
			m36.regs[1] = value;
			break;
		case 0x4102:
			m36.regs[2] = value;
			break;
		case 0x4103:
			m36.regs[3] = value;
			return;
		default:
			if (address >= 0x8000) {
				value = m36.regs [4] >> 4;
				control_bank(info.prg.rom[0].max.banks_32k);
				map_prg_rom_8k(4, 0, value);
				map_prg_rom_8k_update();
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_36(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xE103) {
		case 0x4100:
		case 0x4101:
		case 0x4102:
		case 0x4103:
			return ((openbus & 0xCF) || (m36.regs[4] & 0x30));
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_36(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m36.regs);

	return (EXIT_OK);
}
