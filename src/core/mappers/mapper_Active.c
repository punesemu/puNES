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
#include "save_slot.h"

void map_init_Active(void) {
	EXTCL_CPU_WR_MEM(Active);
	EXTCL_CPU_RD_MEM(Active);
	EXTCL_SAVE_MAPPER(Active);
	mapper.internal_struct[0] = (BYTE *) &active;
	mapper.internal_struct_size[0] = sizeof(active);

	info.mapper.extend_wr = TRUE;

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
		memset(&active, 0x00, sizeof(active));
	} else {
		active.openbus = FALSE;
	}
}
void extcl_cpu_wr_mem_Active(WORD address, BYTE value) {
	BYTE save = value, prg_chip = address >> 10;
	DBWORD bank;

	if ((address < 0x6000) && (address >= 0x4020)) {
		active.prg_ram[address & 0x0003] = value & 0x0F;
		return;
	}

	if (prg_chip == 2) {
		active.openbus = TRUE;
	} else {
		active.openbus = FALSE;

		if (prg_chip == 3) {
			value = (address >> 6) & 0x5F;
		} else {
			value = (address >> 6) & 0x7F;
		}

		if (address & 0x0020) {
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		} else {
			value >>= 1;
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		}
		map_prg_rom_8k_update();
	}


	if (address & 0x2000) {
		mirroring_H();
	} else {
		mirroring_V();
	}

	value = ((address << 2) & 0x3C) | (save & 0x03);
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
BYTE extcl_cpu_rd_mem_Active(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x4020) && (address < 0x6000)) {
		return (active.prg_ram[address & 0x0003]);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Active(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, active.openbus);
	save_slot_ele(mode, slot, active.prg_ram);

	return (EXIT_OK);
}
