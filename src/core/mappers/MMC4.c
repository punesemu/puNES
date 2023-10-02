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

void (*MMC4_prg_fix)(void);
void (*MMC4_prg_swap)(WORD address, WORD value);
void (*MMC4_chr_fix)(void);
void (*MMC4_chr_swap)(WORD address, WORD value);
void (*MMC4_mirroring_fix)(void);

_mmc4 mmc4;

// promemoria
//void map_init_MMC4(void) {
//	EXTCL_AFTER_MAPPER_INIT(MMC4);
//	EXTCL_CPU_WR_MEM(MMC4);
//	EXTCL_SAVE_MAPPER(MMC4);
//	EXTCL_AFTER_RD_CHR(MMC4);
//	EXTCL_UPDATE_R2006(MMC4);
//}

void extcl_after_mapper_init_MMC4(void) {
	MMC4_prg_fix();
	MMC4_chr_fix();
	MMC4_mirroring_fix();
}
void extcl_cpu_wr_mem_MMC4(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0xA000:
			mmc4.prg = value;
			MMC4_prg_fix();
			return;
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
			mmc4.chr[(address - 0xB000) >> 12] = value;
			MMC4_chr_fix();
			return;
		case 0xF000:
			mmc4.mirroring = value;
			MMC4_mirroring_fix();
			return;
	}
}
BYTE extcl_save_mapper_MMC4(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc4.prg);
	save_slot_ele(mode, slot, mmc4.chr);
	save_slot_ele(mode, slot, mmc4.latch);
	save_slot_ele(mode, slot, mmc4.mirroring);
	return (EXIT_OK);
}
void extcl_after_rd_chr_MMC4(UNUSED(BYTE nidx), WORD address) {
	switch (address & 0xFFF0) {
		case 0x0FD0:
			mmc4.latch[0] = 0;
			break;
		case 0x0FE0:
			mmc4.latch[0] = 1;
			break;
		case 0x1FD0:
			mmc4.latch[1] = 2;
			break;
		case 0x1FE0:
			mmc4.latch[1] = 3;
			break;
		default:
			return;
	}
	MMC4_chr_fix();
}
void extcl_update_r2006_MMC4(BYTE nidx, WORD new_r2006, UNUSED(WORD old_r2006)) {
	extcl_after_rd_chr_MMC4(nidx, new_r2006);
}

void init_MMC4(BYTE reset) {
	if (reset >= HARD) {
		memset(&mmc4, 0x00, sizeof(mmc4));

		mmc4.latch[1] = 2;
	}

	MMC4_prg_fix = prg_fix_MMC4_base;
	MMC4_prg_swap = prg_swap_MMC4_base;
	MMC4_chr_fix = chr_fix_MMC4_base;
	MMC4_chr_swap = chr_swap_MMC4_base;
	MMC4_mirroring_fix = mirroring_fix_MMC4_base;
}
void prg_fix_MMC4_base(void) {
	MMC4_prg_swap(0x8000, mmc4.prg);
	MMC4_prg_swap(0xC000, 0xFF);
}
void prg_swap_MMC4_base(WORD address, WORD value) {
	memmap_auto_16k(0, MMCPU(address), value);
}
void chr_fix_MMC4_base(void) {
	MMC4_chr_swap(0x0000, mmc4.chr[mmc4.latch[0]]);
	MMC4_chr_swap(0x1000, mmc4.chr[mmc4.latch[1]]);
}
void chr_swap_MMC4_base(WORD address, WORD value) {
	memmap_auto_4k(0, MMPPU(address), value);
}
void mirroring_fix_MMC4_base(void) {
	if (mmc4.mirroring & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
