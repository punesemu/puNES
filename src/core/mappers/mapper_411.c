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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_mmc3_411(WORD address, WORD value);
void chr_swap_mmc3_411(WORD address, WORD value);

struct _m411 {
	BYTE reg[2];
} m411;

void map_init_411(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(411);
	EXTCL_SAVE_MAPPER(411);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m411;
	mapper.internal_struct_size[0] = sizeof(m411);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m411, 0x00, sizeof(m411));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_411;
	MMC3_chr_swap = chr_swap_mmc3_411;

	m411.reg[0] = 0x80;
	m411.reg[1] = 0x82;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_411(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (cpu.prg_ram_wr_active) {
			m411.reg[address & 0x0001] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_411(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m411.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_411(WORD address, WORD value) {
	WORD base = 0;
	WORD mask = 0;

	if (m411.reg[0] & 0x40) {
		base = ((m411.reg[1] & 0x40) >> 2) | (m411.reg[1] & 0x08) | (m411.reg[0] & 0x05) | ((m411.reg[0] & 0x08) >> 2);
		if (m411.reg[0] & 0x02) {
			base = (base >> 1) << 2;
			mask = 0x03;
			value = (address >> 13) & 0x03;
		} else {
			base = base << 1;
			mask = 0x01;
			value = (address >> 13) & 0x01;
		}
	} else {
		base = ((m411.reg[1] & 0x40) >> 1) | ((m411.reg[1] & 0x08) << 1);
		mask = 0x0F | ((m411.reg[1] & 0x02) << 3);
	}
	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_411(WORD address, WORD value) {
	WORD base = ((m411.reg[0] & 0x10) << 4) | ((m411.reg[1] & 0x04) << 5);
	WORD mask = 0x7F | ((m411.reg[1] & 0x02) << 6);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
