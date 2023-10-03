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

void prg_swap_mmc3_189(WORD address, WORD value);

struct _m189 {
	BYTE reg;
} m189;

void map_init_189(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(189);
	EXTCL_SAVE_MAPPER(189);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m189;
	mapper.internal_struct_size[0] = sizeof(m189);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m189, 0x00, sizeof(m189));
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_189;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_189(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
		case 0x5000:
			if (address & 0x0100) {
				m189.reg = value;
				MMC3_prg_fix();
				MMC3_chr_fix();
			}
			return;
		case 0x6000:
		case 0x7000:
			if (memmap_adr_is_writable(nidx, MMCPU(address))) {
				m189.reg = value;
				MMC3_prg_fix();
				MMC3_chr_fix();
			}
			return;
		default:
			extcl_cpu_wr_mem_MMC3(nidx, address, value);
			return;
	}
}
BYTE extcl_save_mapper_189(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m189.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_189(WORD address, UNUSED(WORD value)) {
	const BYTE slot = (address >> 13) & 0x03;
	WORD base = (m189.reg | (m189.reg >> 4)) << 2;

	prg_swap_MMC3_base(address, (base | slot));
}
