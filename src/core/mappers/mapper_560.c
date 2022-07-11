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

INLINE static void prg_fix_560(void);

struct _m560 {
	BYTE reg;
	BYTE pa13;
	WORD nmt_address;
	BYTE ext_ram[0x400];
} m560;

void map_init_560(void) {
	EXTCL_AFTER_MAPPER_INIT(560);
	EXTCL_CPU_WR_MEM(560);
	EXTCL_SAVE_MAPPER(560);
	EXTCL_WR_NMT(560);
	EXTCL_RD_NMT(560);
	EXTCL_RD_CHR(560);
	mapper.internal_struct[0] = (BYTE *)&m560;
	mapper.internal_struct_size[0] = sizeof(m560);

	memset(&m560, 0x00, sizeof(m560));

	m560.reg = 1;

	info.prg.ram.banks_8k_plus = 1;
}
void extcl_after_mapper_init_560(void) {
	prg_fix_560();
}
void extcl_cpu_wr_mem_560(UNUSED(WORD address), UNUSED(BYTE value)) {
	m560.reg = !m560.reg;
	prg_fix_560();
}
BYTE extcl_save_mapper_560(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m560.reg);
	save_slot_ele(mode, slot, m560.pa13);
	save_slot_ele(mode, slot, m560.nmt_address);
	save_slot_ele(mode, slot, m560.ext_ram);

	return (EXIT_OK);
}
void extcl_wr_nmt_560(WORD address, BYTE value) {
	BYTE bank = (address & 0x0FFF) >> 10;

	if (bank & 0x01) {
		m560.ext_ram[r2006.value & 0x3FF] = value;
	}
	ntbl.bank_1k[bank][address & 0x3FF] = value;
}
BYTE extcl_rd_nmt_560(WORD address) {
    if (!m560.pa13) {
    	m560.nmt_address = r2006.value;
    }
    m560.pa13 = TRUE;
    return (ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF]);
}
BYTE extcl_rd_chr_560(WORD address) {
	WORD base = ((address >> 10) << 9) | ((address & 0x03F0) >> 1) | (address & 0x0007);

	m560.pa13 = FALSE;
	return (m560.reg ?
		chr_rom()[((m560.ext_ram[m560.nmt_address & 0x3FF] << 11) & 0x1F800) | base] :
		chr_rom()[(((address & 0x03FF) << 13) & 0x10000) | base]);
}

INLINE static void prg_fix_560(void) {
	WORD bank;

	bank = m560.reg;
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
