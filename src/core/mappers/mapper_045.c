/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

void prg_swap_mmc3_045(WORD address, WORD value);
void chr_swap_mmc3_045(WORD address, WORD value);

struct _m045 {
	BYTE index;
	BYTE reg[4];
} m045;

void map_init_045(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(045);
	EXTCL_CPU_RD_MEM(045);
	EXTCL_SAVE_MAPPER(045);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m045, sizeof(m045));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m045, 0x00, sizeof(m045));
	m045.reg[2] = 0x0F;

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_045;
	MMC3_chr_swap = chr_swap_mmc3_045;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_045(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m045.reg[3] & 0x40)) {
			m045.reg[m045.index] = value;
			m045.index = (m045.index + 1) & 0x03;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_045(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (dipswitch.used && (address >= 0x5000) && (address <= 0x5FFF)) {
		return (~dipswitch.value & (address & 0xFFF) ? 0x01 : 0x00);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_045(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m045.index);
	save_slot_ele(mode, slot, m045.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_045(WORD address, WORD value) {
	WORD base = m045.reg[1] | ((m045.reg[2] & 0xC0) << 2);
	WORD mask = ~m045.reg[3] & 0x3F;
	BYTE enabled = TRUE;

	if (dipswitch.used) {
		switch (dipswitch.value) {
			case 1:
				enabled = (m045.reg[1] & 0x80) ? FALSE : TRUE;
				break;
			case 2:
				enabled = (m045.reg[2] & 0x40) ? FALSE : TRUE;
				break;
			case 3:
				enabled = (m045.reg[1] & 0x40) ? FALSE : TRUE;
				break;
			case 4:
				enabled = (m045.reg[2] & 0x20) ? FALSE : TRUE;
				break;
		}
	}
	memmap_auto_wp_8k(0, MMCPU(address), (base | (value & mask)), enabled, FALSE);
}
void chr_swap_mmc3_045(WORD address, WORD value) {
	if (!chrrom_size() && (vram_size(0) == S8K)) {
		memmap_vram_1k(0, MMPPU(address), (address >> 10));
	} else {
		WORD base = m045.reg[0] | ((m045.reg[2] & 0xF0) << 4);
		WORD mask = 0xFF >> (~m045.reg[2] & 0x0F);

		chr_swap_MMC3_base(address, (base | (value & mask)));
	}
}
