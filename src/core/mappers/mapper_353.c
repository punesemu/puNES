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

void prg_swap_mmc3_353(WORD address, WORD value);
void chr_swap_mmc3_353(WORD address, WORD value);
void mirroring_fix_mmc3_353(void);

struct _m353 {
	BYTE reg;
} m353;

void map_init_353(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(353);
	EXTCL_SAVE_MAPPER(353);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m353, sizeof(m353));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m353, 0x00, sizeof(m353));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_353;
	MMC3_chr_swap = chr_swap_mmc3_353;
	MMC3_mirroring_fix = mirroring_fix_mmc3_353;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_353(BYTE nidx, WORD address, BYTE value) {
	if ((address & 0x0FFF) == 0x0080) {
		m353.reg = (address >> 13) & 0x03;
		MMC3_prg_fix();
		MMC3_chr_fix();
		MMC3_mirroring_fix();
		return;
	}
	switch (address & 0xE001) {
		case 0x8000:
			mmc3.bank_to_update = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
			MMC3_mirroring_fix();
			return;
		case 0x8001:
			mmc3.reg[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					MMC3_prg_fix();
					MMC3_chr_fix();
					MMC3_mirroring_fix();
					return;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					MMC3_chr_fix();
					MMC3_mirroring_fix();
					return;
				default:
					extcl_cpu_wr_mem_MMC3(nidx, address, value);
					return;
			}
	}
	extcl_cpu_wr_mem_MMC3(nidx, address, value);
}
BYTE extcl_save_mapper_353(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m353.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_353(WORD address, WORD value) {
	WORD base = m353.reg << 5;
	WORD mask = 0x1F;

	if (m353.reg == 2) {
		base |= (mmc3.reg[0] & 0x80 ? 0x10 : 0x00);
		mask >>= 1;
	} else if ((m353.reg == 3) && !(mmc3.reg[0] & 0x80) && (address >= 0xC000)) {
		base = 0x70;
		mask = 0x0F;
		value = mmc3.reg[address >> 13];
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_353(WORD address, WORD value) {
	if ((m353.reg == 2) && (mmc3.reg[0] & 0x80) && vram_size(0)) {
		memmap_vram_1k(0, MMPPU(address), address >> 10);
	} else {
		WORD base = m353.reg << 7;
		WORD mask = 0x7F;

		memmap_auto_1k(0, MMPPU(address), ((base & ~mask) | (value & mask)));
	}
}
void mirroring_fix_mmc3_353(void) {
	if (!m353.reg) {
		if (!(mmc3.reg[0] & 0x80)) {
			memmap_nmt_1k(0, MMPPU(0x2000), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2400), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2800), ((mmc3.reg[1] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2C00), ((mmc3.reg[1] >> 7) ^ 0x01));

			memmap_nmt_1k(0, MMPPU(0x3000), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3400), ((mmc3.reg[0] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3800), ((mmc3.reg[1] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3C00), ((mmc3.reg[1] >> 7) ^ 0x01));
		} else {
			memmap_nmt_1k(0, MMPPU(0x2000), ((mmc3.reg[2] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2400), ((mmc3.reg[3] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2800), ((mmc3.reg[4] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x2C00), ((mmc3.reg[5] >> 7) ^ 0x01));

			memmap_nmt_1k(0, MMPPU(0x3000), ((mmc3.reg[2] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3400), ((mmc3.reg[3] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3800), ((mmc3.reg[4] >> 7) ^ 0x01));
			memmap_nmt_1k(0, MMPPU(0x3C00), ((mmc3.reg[5] >> 7) ^ 0x01));
		 }
	} else {
		mirroring_fix_MMC3_base();
	}
}
