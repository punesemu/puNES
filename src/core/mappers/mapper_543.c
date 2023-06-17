/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

void prg_swap_mmc1_543(WORD address, WORD value);
void chr_swap_mmc1_543(WORD address, WORD value);
void wram_fix_mmc1_543(void);

struct _m543 {
	BYTE reg;
	BYTE accumulator;
	BYTE shift;
} m543;

void map_init_543(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(543);
	EXTCL_SAVE_MAPPER(543);
	mapper.internal_struct[0] = (BYTE *)&m543;
	mapper.internal_struct_size[0] = sizeof(m543);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&m543, 0x00, sizeof(m543));

	init_MMC1(MMC1B, HARD);
	MMC1_prg_swap = prg_swap_mmc1_543;
	MMC1_chr_swap = chr_swap_mmc1_543;
	MMC1_wram_fix = wram_fix_mmc1_543;

	// per far avviare 1996 無敵智カ卡 5-in-1 (CH-501).nes
	mmc1.reg[3] = 0x0E;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_543(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m543.accumulator |= (((value & 0x08) >> 3) << m543.shift);
		if (m543.shift++ == 3) {
			m543.reg = m543.accumulator;
			m543.accumulator = m543.shift = 0;
			MMC1_prg_fix();
			MMC1_chr_fix();
			MMC1_wram_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC1(address, value);
	}
}
BYTE extcl_save_mapper_543(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m543.reg);
	save_slot_ele(mode, slot, m543.accumulator);
	save_slot_ele(mode, slot, m543.shift);
	return (extcl_save_mapper_MMC1(mode, slot, fp));
}

void prg_swap_mmc1_543(WORD address, WORD value) {
	prg_swap_MMC1_base(address, ((m543.reg << 4) | (value & 0x0F)));
}
void chr_swap_mmc1_543(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (value & 0x07));
}
void wram_fix_mmc1_543(void) {
	WORD bank = m543.reg & 0x02
		? 0x04 | ((m543.reg & 0x04) >> 1) | (m543.reg & 0x01)
		: ((m543.reg & 0x01) << 1) | ((mmc1.reg[1] & 0x08) >> 3);

	MMC1_wram_swap(0x6000, bank);
}
