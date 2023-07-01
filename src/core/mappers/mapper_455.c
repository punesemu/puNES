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
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_mmc3_455(WORD address, WORD value);
void chr_swap_mmc3_455(WORD address, WORD value);

struct _m455 {
	WORD reg[2];
} m455;

void map_init_455(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(455);
	EXTCL_SAVE_MAPPER(455);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m455;
	mapper.internal_struct_size[0] = sizeof(m455);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	memset(&m455, 0x00, sizeof(m455));
	m455.reg[1] = 1;

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_455;
	MMC3_chr_swap = chr_swap_mmc3_455;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_455(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if (address & 0x0100) {
			m455.reg[0] = address;
			m455.reg[1] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_455(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m455.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_455(WORD address, WORD value) {
	WORD base = ((m455.reg[1] & 0x40) >> 2) | ((m455.reg[0] & 0x04) << 1) | ((m455.reg[1] & 0x1C) >> 2);
	WORD mask = 0x1F >> !(m455.reg[0] & 0x01);

	if (m455.reg[1] & 0x01) {
		WORD nrom = (m455.reg[1] & 0x02) >> 1;

		base = (address & 0x4000 ? base | nrom: base & ~nrom) << 1;
		mask = 0x01;
		value = (address >> 13) & 0x01;
	} else {
		base <<= 1;
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_455(WORD address, WORD value) {
	WORD base = (((m455.reg[1] & 0x40) >> 2) | ((m455.reg[1] & 0x1C) >> 2) | ((m455.reg[0] & 0x0004) << 1)) << 4;
	WORD mask = 0xFF >> !(m455.reg[0] & 0x02);

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
