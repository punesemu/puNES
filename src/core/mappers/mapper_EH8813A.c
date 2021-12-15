/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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
#include "mem_map.h"
#include "save_slot.h"

struct _eh8813a {
	WORD address;
	BYTE hwmode;
} eh88131a;

void map_init_EH8813A(void) {
	EXTCL_CPU_WR_MEM(EH8813A);
	EXTCL_CPU_RD_MEM(EH8813A);
	EXTCL_SAVE_MAPPER(EH8813A);
	mapper.internal_struct[0] = (BYTE *)&eh88131a;
	mapper.internal_struct_size[0] = sizeof(eh88131a);

	if (info.reset >= HARD) {
		eh88131a.address = 0;
		eh88131a.hwmode = 0;
	}

	if (info.reset == RESET) {
		eh88131a.address = 0;
		eh88131a.hwmode = (eh88131a.hwmode + 1) & 0x0F;
	}

	map_prg_rom_8k(4, 0, 0);
	map_chr_bank_1k_reset();
	mirroring_V();

	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_EH8813A(WORD address, BYTE value) {
	if ((eh88131a.address & 0x100) == 0) {
		DBWORD bank;

		eh88131a.address = address & 0x01FF;

		value &= 0x0F;
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

		value = eh88131a.address & 0x07;

		if (eh88131a.address & 0x80) {
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
}
BYTE extcl_cpu_rd_mem_EH8813A(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address < 0x8000) {
		return (openbus);
	}

	if (eh88131a.address & 0x40) {
		address = (address & 0xFFF0) + eh88131a.hwmode;
	}

	return (prg_rom_rd(address));
}
BYTE extcl_save_mapper_EH8813A(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, eh88131a.address);
	save_slot_ele(mode, slot, eh88131a.hwmode);

	return (EXIT_OK);
}
