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

void chr_swap_199(WORD address, WORD value);

INLINE static void prg_5000_fix(void);

struct _m199tmp {
	BYTE *prg_5000;
} m199tmp;

void map_init_199(void) {
	EXTCL_AFTER_MAPPER_INIT(199);
	EXTCL_CPU_WR_MEM(199);
	EXTCL_CPU_RD_MEM(199);
	EXTCL_SAVE_MAPPER(199);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();
	MMC3_chr_swap = chr_swap_199;

	if (info.prg.ram.banks_8k_plus < 2) {
		info.prg.ram.banks_8k_plus = 2;
		info.prg.ram.bat.banks = 2;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_199(void) {
	extcl_after_mapper_init_MMC3();
	prg_5000_fix();
}
void extcl_cpu_wr_mem_199(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m199tmp.prg_5000[address & 0x0FFF] = value;
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_199(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (m199tmp.prg_5000[address & 0x0FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_199(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix(mmc3.bank_to_update);
		prg_5000_fix();
	}

	return (EXIT_OK);
}

void chr_swap_199(WORD address, WORD value) {
	const BYTE slot = address >> 10;

	value = slot;
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[slot] = chr_pnt(value << 10);
}

INLINE static void prg_5000_fix(void) {
	m199tmp.prg_5000 = &prg.ram_plus[0x2000 & ((info.prg.ram.banks_8k_plus * 0x2000) - 1)];
}