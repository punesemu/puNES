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

void prg_swap_401(WORD address, WORD value);
void chr_swap_401(WORD address, WORD value);

static BYTE dipswitch_401[] = { 7, 1, 2, 4, 3, 5, 6, 0 };

struct _m401 {
	BYTE index;
	BYTE reg[4];
} m401;
struct _m401tmp {
	BYTE index;
	BYTE dipswitch;
} m401tmp;

void map_init_401(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(401);
	EXTCL_CPU_RD_MEM(401);
	EXTCL_SAVE_MAPPER(401);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m401;
	mapper.internal_struct_size[0] = sizeof(m401);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m401, 0x00, sizeof(m401));

	init_MMC3();
	MMC3_prg_swap = prg_swap_401;
	MMC3_chr_swap = chr_swap_401;

	if (info.reset == RESET) {
		m401tmp.index = (m401tmp.index + 1) & 0x07;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m401tmp.index = 0;
	}

	m401tmp.dipswitch = dipswitch_401[m401tmp.index];

	m401.reg[2] = 0x0F;

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_401(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if ((cpu.prg_ram_wr_active) && !(m401.reg[3] & 0x40)) {
			m401.reg[m401.index] = value;
			m401.index = (m401.index + 1) & 0x03;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_401(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0x8000) {
		if ((m401tmp.dipswitch & 0x01) && (m401.reg[1] & 0x80)) {
			return (m401tmp.dipswitch);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_401(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m401.index);
	save_slot_ele(mode, slot, m401.reg);
	save_slot_ele(mode, slot, m401tmp.index);
	save_slot_ele(mode, slot, m401tmp.dipswitch);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_401(WORD address, WORD value) {
	WORD base = (m401.reg[1] & 0x1F) | (m401.reg[2] & 0x80) |
		(m401tmp.dipswitch & 0x02 ? m401.reg[2] & 0x20 : (m401.reg[1] & 0x40) >> 1) |
		(m401tmp.dipswitch & 0x04 ? m401.reg[2] & 0x40 : (m401.reg[1] & 0x20) << 1);
	WORD mask = ~m401.reg[3] & 0x1F;

	prg_swap_MMC3(address, (base | (value & mask)));
}
void chr_swap_401(WORD address, WORD value) {
	WORD base = m401.reg[0] | (m401.reg[2] & 0xF0) << 4;
	WORD mask = 0xFF >> (~m401.reg[2] & 0x0F);

	chr_swap_MMC3(address, (base | (value & mask)));
}
