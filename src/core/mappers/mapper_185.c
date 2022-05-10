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
#include "save_slot.h"

struct _m185 {
	BYTE reg;
	BYTE ppu_read_count;
} m185;

void map_init_185() {
	EXTCL_CPU_WR_MEM(185);
	EXTCL_SAVE_MAPPER(185);
	EXTCL_RD_R2007(185);
	EXTCL_RD_CHR(185);
	mapper.internal_struct[0] = (BYTE *)&m185;
	mapper.internal_struct_size[0] = sizeof(m185);

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}

	memset(&m185, 0x00, sizeof(m185));
}
void extcl_cpu_wr_mem_185(WORD address, BYTE value) {
	DBWORD bank;

	// bus conflict
	value &= prg_rom_rd(address);

	m185.reg = value;
	control_bank(info.chr.rom.max.banks_8k)
	bank = value << 13;

	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
BYTE extcl_save_mapper_185(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m185.reg);
	save_slot_ele(mode, slot, m185.ppu_read_count);

	return (EXIT_OK);
}
BYTE extcl_rd_chr_185(WORD address) {
	if ((info.mapper.submapper & 0x0C) == 0x04) {
		return (((m185.reg & 0x03) == (info.mapper.submapper & 0x03)) ? chr.bank_1k[address >> 10][address & 0x3FF] : 0xFF);
	}
	return (m185.ppu_read_count >= 2) ? chr.bank_1k[address >> 10][address & 0x3FF] : 0xFF;
}
void extcl_rd_r2007_185(void) {
	if (m185.ppu_read_count < 2) {
		m185.ppu_read_count++;
	}
}
