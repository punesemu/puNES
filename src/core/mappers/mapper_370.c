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

void prg_swap_mmc3_370(WORD address, WORD value);
void chr_swap_mmc3_370(WORD address, WORD value);
void mirroring_fix_mmc3_370(void);

struct _m370 {
	BYTE reg;
} m370;

void map_init_370(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(370);
	EXTCL_CPU_RD_MEM(370);
	EXTCL_SAVE_MAPPER(370);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m370, sizeof(m370));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m370, 0x00, sizeof(m370));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_370;
	MMC3_chr_swap = chr_swap_mmc3_370;
	MMC3_mirroring_fix = mirroring_fix_mmc3_370;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_370(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m370.reg = address & 0xFF;
		MMC3_prg_fix();
		MMC3_chr_fix();
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_370(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((dipswitch.value & 0x80) | (openbus & 0x7F));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_370(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m370.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_370(WORD address, WORD value) {
	WORD base = (m370.reg & 0x38) << 1;
	WORD mask = 0x1F >> ((m370.reg & 0x20) >> 5);

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_370(WORD address, WORD value) {
	WORD base = (m370.reg & 0x07) << 7;
	WORD mask = 0xFF >> ((~m370.reg & 0x04) >> 2);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
void mirroring_fix_mmc3_370(void) {
	if ((m370.reg & 0x07) == 0x01) {
		if (mmc3.bank_to_update & 0x80) {
			memmap_nmt_1k(0, MMPPU(0x2000), ((mmc3.reg[2] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2400), ((mmc3.reg[3] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2800), ((mmc3.reg[4] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2C00), ((mmc3.reg[5] >> 7) ^ 0x01));

			memmap_nmt_1k(0, MMPPU(0x3000), ((mmc3.reg[2] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3400), ((mmc3.reg[3] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3800), ((mmc3.reg[4] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3C00), ((mmc3.reg[5] >> 7) ^ 0x01));
		} else {
			memmap_nmt_1k(0, MMPPU(0x2000), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2400), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2800), ((mmc3.reg[1] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2C00), ((mmc3.reg[1] >> 7) ^ 0x01));

			memmap_nmt_1k(0, MMPPU(0x3000), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3400), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3800), ((mmc3.reg[1] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3C00), ((mmc3.reg[1] >> 7) ^ 0x01));
		}
		return;
	}
	mirroring_fix_MMC3_base();
}
