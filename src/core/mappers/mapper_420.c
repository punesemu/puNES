/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

void prg_swap_mmc3_420(WORD address, WORD value);
void chr_swap_mmc3_420(WORD address, WORD value);

struct _m420 {
	BYTE reg[4];
} m420;

void map_init_420(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(420);
	EXTCL_SAVE_MAPPER(420);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m420, sizeof(m420));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m420, 0x00, sizeof(m420));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_420;
	MMC3_chr_swap = chr_swap_mmc3_420;

	m420.reg[2] = 0x0F;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_420(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			m420.reg[address & 0x03] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_420(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m420.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_420(WORD address, WORD value) {
	WORD base = 0;
	WORD mask = 0;

	if (m420.reg[0] & 0x80) {
		base = (((m420.reg[3] & 0x20) >> 2) | ((m420.reg[0] & 0x0E) >> 1)) << 2;
		mask = 0x03;
		value = (address >> 13) & 0x03;
	} else {
		base = (m420.reg[3] & 0x04) << 3;
		mask = 0x3F >> (m420.reg[0] & 0x20 ? 2 : ((m420.reg[3] & 0x20) >> 5));
	}
	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_420(WORD address, WORD value) {
	WORD base = ((m420.reg[1] & 0x80) << 1) | ((m420.reg[1] & 0x04) << 5);
	WORD mask = 0xFF >> ((m420.reg[1] & 0x80) >> 7);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
