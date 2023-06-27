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

void prg_swap_mmc3_401(WORD address, WORD value);
void chr_swap_mmc3_401(WORD address, WORD value);

struct _m401 {
	BYTE index;
	BYTE reg[4];
} m401;

void map_init_401(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(401);
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

	if (info.reset >= HARD) {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	memset(&m401, 0x00, sizeof(m401));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_401;
	MMC3_chr_swap = chr_swap_mmc3_401;

	m401.reg[2] = 0x0F;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_401(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m401.reg[3] & 0x40) && memmap_adr_is_writable(MMCPU(address))) {
			m401.reg[m401.index] = value;
			m401.index = (m401.index + 1) & 0x03;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_401(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m401.index);
	save_slot_ele(mode, slot, m401.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_401(WORD address, WORD value) {
	if ((dipswitch.value & 0x01) && (m401.reg[1] & 0x80)) {
		memmap_disable_8k(MMCPU(address));
	} else {
		WORD base = (m401.reg[1] & 0x1F) | (m401.reg[2] & 0x80) |
			(dipswitch.value & 0x02 ? m401.reg[2] & 0x20 : (m401.reg[1] & 0x40) >> 1) |
			(dipswitch.value & 0x04 ? m401.reg[2] & 0x40 : (m401.reg[1] & 0x20) << 1);
		WORD mask = ~m401.reg[3] & 0x1F;

		prg_swap_MMC3_base(address, (base | (value & mask)));
	}
}
void chr_swap_mmc3_401(WORD address, WORD value) {
	WORD base = m401.reg[0] | (m401.reg[2] & 0xF0) << 4;
	WORD mask = 0xFF >> (~m401.reg[2] & 0x0F);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
