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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_398(void);
INLINE static void chr_fix_398(void);

struct _m398 {
	BYTE reg;
	BYTE prg[2];
	BYTE swap;
	BYTE chr_slot;
} m398;

void map_init_398(void) {
	map_init_VRC4(VRC4UNL);

	EXTCL_AFTER_MAPPER_INIT(398);
	EXTCL_CPU_WR_MEM(398);
	EXTCL_SAVE_MAPPER(398);
	EXTCL_RD_CHR(398);
	mapper.internal_struct[0] = (BYTE *)&m398;
	mapper.internal_struct_size[0] = sizeof(m398);
	mapper.internal_struct[1] = (BYTE *)&vrc4;
	mapper.internal_struct_size[1] = sizeof(vrc4);

	memset(&m398, 0x00, sizeof(m398));

	m398.reg = 0xC0;
	m398.prg[0] = 0;
	m398.prg[1] = 1;
}
void extcl_after_mapper_init_398(void) {
	prg_fix_398();
	chr_fix_398();
}
void extcl_cpu_wr_mem_398(WORD address, BYTE value) {
	BYTE tmp = address & 0xFF;

	if (tmp != m398.reg) {
		m398.reg = tmp;
	}

	switch (address_VRC4(address)) {
		case 0x8000:
			m398.prg[0] = value;
			break;
		case 0xA000:
			m398.prg[1] = value;
			break;
		case 0x9002:
		case 0x9003:
			m398.swap = value & 0x02;
			break;
		default:
			extcl_cpu_wr_mem_VRC4(address, value);
			break;
	}
	prg_fix_398();
	chr_fix_398();
}
BYTE extcl_save_mapper_398(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m398.reg);
	save_slot_ele(mode, slot, m398.prg);
	save_slot_ele(mode, slot, m398.swap);
	save_slot_ele(mode, slot, m398.chr_slot);
	extcl_save_mapper_VRC4(mode, slot, fp);

	return (EXIT_OK);
}
BYTE extcl_rd_chr_398(WORD address) {
	m398.chr_slot = address >> 10;
	if (m398.reg & 0x80) {
		prg_fix_398();
		chr_fix_398();
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}

INLINE static void prg_fix_398(void) {
	WORD bank;

	if (m398.reg & 0x80) {
		bank = ((m398.reg & 0xC0) >> 5) | ((vrc4.chr_rom_bank[m398.chr_slot] & 0x04) >> 2);
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		bank = m398.prg[0] & 0x0F;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0 ^ m398.swap, bank);

		bank = m398.prg[1] & 0x0F;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, bank);

		bank = 0xFE & 0x0F;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2 ^ m398.swap, bank);

		bank = 0xFF & 0x0F;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_398(void) {
	DBWORD bank;

	if (m398.reg & 0x80) {
		bank = 0x40 | ((m398.reg & 0x40) >> 3) | (vrc4.chr_rom_bank[m398.chr_slot] & 0x07);

		_control_bank(bank, info.chr.rom.max.banks_8k)
		bank <<= 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}
