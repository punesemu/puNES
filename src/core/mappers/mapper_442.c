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
#include "ppu.h"
#include "save_slot.h"

// TODO : aggiungere l'emulazione della tastiera.

INLINE static void prg_fix_442(void);
INLINE static void mode1_bpp(WORD address);
INLINE static BYTE mode1_bpp_rd(WORD address);

struct _m442 {
	BYTE reg[8];
	BYTE pa0;
	BYTE pa9;
	BYTE pa13;
} m442;

void map_init_442(void) {
	EXTCL_AFTER_MAPPER_INIT(442);
	EXTCL_CPU_WR_MEM(442);
	EXTCL_SAVE_MAPPER(442);
	EXTCL_WR_NMT(442);
	EXTCL_RD_NMT(442);
	EXTCL_WR_CHR(442);
	EXTCL_RD_CHR(442);
	mapper.internal_struct[0] = (BYTE *)&m442;
	mapper.internal_struct_size[0] = sizeof(m442);

	memset(&m442, 0x00, sizeof(m442));

	info.prg.ram.banks_8k_plus = 1;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_442(void) {
	prg_fix_442();
}
void extcl_cpu_wr_mem_442(WORD address, UNUSED(BYTE value)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m442.reg[((address & 0x0700) >> 8)] = value;
		prg_fix_442();
	}
}
BYTE extcl_save_mapper_442(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m442.reg);
	save_slot_ele(mode, slot, m442.pa0);
	save_slot_ele(mode, slot, m442.pa9);
	save_slot_ele(mode, slot, m442.pa13);

	return (EXIT_OK);
}
void extcl_wr_nmt_442(WORD address, BYTE value) {
	mode1_bpp(r2006.value);
	ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF] = value;
}
BYTE extcl_rd_nmt_442(WORD address) {
	mode1_bpp(r2006.value);
	if ((m442.reg[0] & 0x80) && !m442.pa13) {
		return (mode1_bpp_rd(r2006.value));
	}
    return (ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF]);
}

void extcl_wr_chr_442(WORD address, BYTE value) {
	mode1_bpp(r2006.value);
	if (mapper.write_vram) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
BYTE extcl_rd_chr_442(WORD address) {
	mode1_bpp(r2006.value);
	if ((m442.reg[0] & 0x80) && !m442.pa13) {
		return (mode1_bpp_rd(r2006.value));
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}

INLINE static void prg_fix_442(void) {
	WORD bank;

	bank = ((m442.reg[0] & 0x40) >> 1) | (m442.reg[0] & 0x1F);
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void mode1_bpp(WORD address) {
	BYTE pa13 = (address & 0x2000) >> 13;

	if (!m442.pa13 && pa13) {
		m442.pa0 = !!(address & 0x001);
		m442.pa9 = !!(address & 0x200);
	}
	m442.pa13 = pa13;
}
INLINE static BYTE mode1_bpp_rd(WORD address) {
	address = (m442.pa9 << 12) | (address & 0x0FF7) | (m442.pa0 << 3);
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
