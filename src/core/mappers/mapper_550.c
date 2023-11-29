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

void prg_swap_mmc1_550(WORD address, WORD value);
void chr_swap_mmc1_550(WORD address, WORD value);

struct _m550 {
	BYTE reg[2];
} m550;

void map_init_550(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(550);
	EXTCL_SAVE_MAPPER(550);
	map_internal_struct_init((BYTE *)&m550, sizeof(m550));
	map_internal_struct_init((BYTE *)&mmc1, sizeof(mmc1));

	memset(&m550, 0x00, sizeof(m550));

	init_MMC1(MMC1A, HARD);
	MMC1_prg_swap = prg_swap_mmc1_550;
	MMC1_chr_swap = chr_swap_mmc1_550;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_550(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x7000) && (address <= 0x7FFF)) {
		if (!(m550.reg[0] & 0x08)) {
			m550.reg[0] = address & 0x0F;
			MMC1_prg_fix();
			MMC1_chr_fix();
			MMC1_mirroring_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		m550.reg[1] = value;
		if ((m550.reg[0] & 0x06) == 0x06) {
			extcl_cpu_wr_mem_MMC1(nidx, address, value);
			return;
		}
		MMC1_prg_fix();
		MMC1_chr_fix();
		MMC1_mirroring_fix();
	}
}
BYTE extcl_save_mapper_550(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m550.reg);
	return (extcl_save_mapper_MMC1(mode, slot, fp));
}

void prg_swap_mmc1_550(WORD address, WORD value) {
	if ((m550.reg[0] & 0x06) == 0x06) {
		value = (m550.reg[0] << 2) | (value & 0x07);
	} else {
		value = (((m550.reg[0] << 1) | (m550.reg[1] >> 4)) << 1) | ((address >> 14) & 0x01);
	}
	prg_swap_MMC1_base(address, value);
}
void chr_swap_mmc1_550(WORD address, WORD value) {
	if ((m550.reg[0] & 0x06) == 0x06) {
		value = 0x18 | (value & 0x07);
	} else {
		value = ((((m550.reg[0] << 1) & 0x0C) | (m550.reg[1] & 0x03)) << 1) | ((address >> 12) & 0x01);
	}
	chr_swap_MMC1_base(address, value);
}
