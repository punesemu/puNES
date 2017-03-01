/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

BYTE mask, state;

void map_init_CNROM() {
	EXTCL_CPU_WR_MEM(CNROM);

	mask = state = 0x00;

	/*
	 * "Cybernoid - The Fighting Machine (U) [!].nes" vuole
	 * la gestione del bus conflict per funzionare correttamente.
	 */

	if ((info.id >= CNROM_26CE27CE) && (info.id <= CNROM_26NCE27NCE)) {
		EXTCL_RD_CHR(CNROM);
		EXTCL_SAVE_MAPPER(CNROM);
		mapper.internal_struct[0] = (BYTE *) &cnrom_2627;
		mapper.internal_struct_size[0] = sizeof(cnrom_2627);

		memset(&cnrom_2627, 0x00, sizeof(cnrom_2627));
		mask = 0x03;

		switch (info.id) {
			case CNROM_26CE27CE:
				state = 0x03;
				break;
			case CNROM_26CE27NCE:
				state = 0x01;
				break;
			case CNROM_26NCE27CE:
				state = 0x02;
				break;
			case CNROM_26NCE27NCE:
				state = 0x00;
				break;
		}
	}
}
void extcl_cpu_wr_mem_CNROM(WORD address, BYTE value) {
	DBWORD bank;

	/* bus conflict */
	if (info.mapper.submapper == CNROM_CNFL) {
		value &= prg_rom_rd(address);
	}

	if (mask) {
		if ((value & mask) == state) {
			cnrom_2627.chr_rd_enable = FALSE;
		} else {
			cnrom_2627.chr_rd_enable = TRUE;
		}
		value &= ~mask;
	}

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
BYTE extcl_save_mapper_CNROM(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, cnrom_2627.chr_rd_enable);

	return (EXIT_OK);
}
BYTE extcl_rd_chr_CNROM(WORD address) {
	if (cnrom_2627.chr_rd_enable == TRUE) {
		return (0xFF);
	}

	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
