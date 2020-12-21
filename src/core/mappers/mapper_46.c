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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

struct _m46 {
	BYTE prg;
	BYTE chr;
} m46;

void map_init_46(void) {
	EXTCL_CPU_WR_MEM(46);
	EXTCL_SAVE_MAPPER(46);
	mapper.internal_struct[0] = (BYTE *) &m46;
	mapper.internal_struct_size[0] = sizeof(m46);

	if (info.reset >= HARD) {
		memset(&m46, 0x00, sizeof(m46));

		map_prg_rom_8k(4, 0, 0);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_46(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	if (address >= 0x8000) {
		value = (m46.prg & 0x1E) | (save & 0x01);
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		m46.prg = value;

		value = (m46.chr & 0x78) | ((save >> 4) & 0x07);
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
		m46.chr = value;

		return;
	}

	if (address >= 0x6000) {
		value = (m46.prg & 0x01) | ((save << 1) & 0x1E);
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		m46.prg = value;

		value = (m46.chr & 0x07) | ((save >> 1) & 0x78);
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
		m46.chr = value;

		return;
	}
}
BYTE extcl_save_mapper_46(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m46.prg);
	save_slot_ele(mode, slot, m46.chr);
	return (EXIT_OK);
}
