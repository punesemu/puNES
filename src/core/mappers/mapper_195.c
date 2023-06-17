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

void prg_swap_mmc3_195(WORD address, WORD value);
void chr_swap_mmc3_195(WORD address, WORD value);
void wram_fix_mmc3_195(void);

INLINE static BYTE chr_bank_mmc3(WORD address);

struct _m195 {
	struct _m195_chr {
		BYTE mask;
		BYTE compare;
	} chr;
} m195;

void map_init_195(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(195);
	EXTCL_SAVE_MAPPER(195);
	EXTCL_WR_CHR(195);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m195;
	mapper.internal_struct_size[0] = sizeof(m195);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&irqA12, 0x00, sizeof(irqA12));

		m195.chr.mask = 0xFC;
		m195.chr.compare = 0x00;
	}

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_195;
	MMC3_chr_swap = chr_swap_mmc3_195;
	MMC3_wram_fix = wram_fix_mmc3_195;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_195(WORD address, BYTE value) {
	if ((address & 0xE001) == 0xA001) {
		return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_195(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m195.chr.mask);
	save_slot_ele(mode, slot, m195.chr.compare);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}
void extcl_wr_chr_195(WORD address, UNUSED(BYTE value)) {
	const BYTE bank = chr_bank_mmc3(address);
	static const BYTE masks[8] = {
		/* $80 */ 0x28,
		/* $82 */ 0x00,
		/* $88 */ 0x4C,
		/* $8A */ 0x64,
		/* $C0 */ 0x46,
		/* $C2 */ 0x7C,
		/* $C8 */ 0x0A,
		/* $CA */ 0xFF,
	};

	if (bank & 0x80) {
		BYTE index = ((bank & 0x40) >> 4) | ((bank & 0x08) >> 2) | ((bank & 0x02) >> 1);

		// CHRB~[1Z.D L.L.]
		//       || | | |
		//       |+-|-+-+---------------- Select first bank and size of CHR RAM:
		//       || |                      $80 = $28-$2B
		//       || |                      $82 = $00-$03
		//       || |                      $88 = $4C-$4F
		//       || |                      $8A = $64-$67
		//       || |                      $C0 = $46-$47
		//       || |                      $C2 = $7C-$7D
		//       || |                      $C8 = $0A-$0B
		//       || |                      $CA = only CHR ROM
		//       || +-------------------- If 1, ignore above and always enable CHR ROM / disable CHR RAM
		//       |+---------------------- Number of banks of CHR RAM, 0=4KiB, 1=2KiB
		//       +----------------------- Must be 1

		// 天使之翼 2, 足球小将: Waixing's Chinese translation of Tecmo's Captain Tsubasa Vol. II: Super Strike
		if (info.crc32.prg == 0x7BEAEBDB) {
			m195.chr.mask = 0xFC;
			m195.chr.compare = masks[1];
		} else {
			if (bank & 0x10) {
				m195.chr.mask = 0x00;
				m195.chr.compare = 0xFF;
			} else {
				m195.chr.mask = 0xFC | ((bank & 0x40) >> 5);
				m195.chr.compare = masks[index];
			}
		}
		MMC3_chr_fix();
	}
	chr_wr(address, value);
}

void prg_swap_mmc3_195(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_195(WORD address, WORD value) {
	if (((value & m195.chr.mask) == m195.chr.compare) && vram_size()){
		memmap_vram_1k(MMPPU(address), value);
	} else {
		chr_swap_MMC3_base(address, value);
	}
}
void wram_fix_mmc3_195(void) {
	memmap_auto_4k(MMCPU(0x5000), 2);
	wram_fix_MMC3_base();
}

INLINE static BYTE chr_bank_mmc3(WORD address) {
	const BYTE slot = address >> 10;
	const BYTE index = mmc3.bank_to_update & 0x80 ? slot ^ 0x04 : slot;

	if (index & 0x04) {
		return (mmc3.reg[index - 2]);
	}
	return ((mmc3.reg[index >> 1] & ~1) | (index & 0x01));
}
