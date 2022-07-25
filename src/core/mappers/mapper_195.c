/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_195(BYTE value);
INLINE static void prg_swap_195(WORD address, WORD value);
INLINE static void chr_fix_195(BYTE value);
INLINE static void chr_swap_195(WORD address, WORD value);
INLINE static BYTE chr_bank_mmc3(WORD address);

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
struct _m195 {
	WORD mmc3[8];
	struct _m195_chr {
		BYTE mask;
		BYTE compare;
	} chr;
} m195;

void map_init_195(void) {
	EXTCL_AFTER_MAPPER_INIT(195);
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

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m195, 0x00, sizeof(m195));

	m195.chr.mask = 0xFC;
	m195.chr.compare = 0x00;

	m195.mmc3[0] = 0;
	m195.mmc3[1] = 2;
	m195.mmc3[2] = 4;
	m195.mmc3[3] = 5;
	m195.mmc3[4] = 6;
	m195.mmc3[5] = 7;
	m195.mmc3[6] = 0;
	m195.mmc3[7] = 0;

	map_chr_ram_extra_init(0x2000);

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_195(void) {
	prg_fix_195(mmc3.bank_to_update);
	chr_fix_195(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_195(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				prg_fix_195(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				chr_fix_195(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			m195.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					chr_swap_195(cbase ^ 0x0000, value & (~1));
					chr_swap_195(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					chr_swap_195(cbase ^ 0x0800, value & (~1));
					chr_swap_195(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					chr_swap_195(cbase ^ 0x1000, value);
					return;
				case 3:
					chr_swap_195(cbase ^ 0x1400, value);
					return;
				case 4:
					chr_swap_195(cbase ^ 0x1800, value);
					return;
				case 5:
					chr_swap_195(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_195(0xC000, value);
					} else {
						prg_swap_195(0x8000, value);
					}
					return;
				case 7:
					prg_swap_195(0xA000, value);
					return;
			}
			return;
		}
		case 0xA001:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_195(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m195.mmc3);
	save_slot_ele(mode, slot, m195.chr.mask);
	save_slot_ele(mode, slot, m195.chr.compare);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		chr_fix_195(mmc3.bank_to_update);
	}

	return (EXIT_OK);
}
void extcl_wr_chr_195(WORD address, BYTE value) {
	BYTE slot = address >> 10;
	BYTE bank = chr_bank_mmc3(address);

	if (bank & 0x80) {
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
		BYTE index = ((bank & 0x40) >> 4) | ((bank & 0x08) >> 2) | ((bank & 0x02) >> 1);

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
		chr_fix_195(mmc3.bank_to_update);
	}

	if ((chr.bank_1k[slot] >= chr.extra.data) && (chr.bank_1k[slot] < (chr.extra.data + chr.extra.size))) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_195(BYTE value) {
	if (value & 0x40) {
		prg_swap_195(0x8000, ~1);
		prg_swap_195(0xC000, m195.mmc3[6]);
	} else {
		prg_swap_195(0x8000, m195.mmc3[6]);
		prg_swap_195(0xC000, ~1);
	}
	prg_swap_195(0xA000, m195.mmc3[7]);
	prg_swap_195(0xE000, ~0);
}
INLINE static void prg_swap_195(WORD address, WORD value) {
	value &= 0x3F;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_195(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_195(cbase ^ 0x0000, m195.mmc3[0] & (~1));
	chr_swap_195(cbase ^ 0x0400, m195.mmc3[0] |   1);
	chr_swap_195(cbase ^ 0x0800, m195.mmc3[1] & (~1));
	chr_swap_195(cbase ^ 0x0C00, m195.mmc3[1] |   1);
	chr_swap_195(cbase ^ 0x1000, m195.mmc3[2]);
	chr_swap_195(cbase ^ 0x1400, m195.mmc3[3]);
	chr_swap_195(cbase ^ 0x1800, m195.mmc3[4]);
	chr_swap_195(cbase ^ 0x1C00, m195.mmc3[5]);
}
INLINE static void chr_swap_195(WORD address, WORD value) {
	BYTE slot = address >> 10;

	if ((value & m195.chr.mask) == m195.chr.compare) {
		control_bank(info.chr.ram.max.banks_1k)
		chr.bank_1k[slot] = &chr.extra.data[value << 10];
	} else {
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[slot] = chr_pnt(value << 10);
	}
}
INLINE static BYTE chr_bank_mmc3(WORD address) {
	BYTE slot = address >> 10;
	BYTE index = mmc3.bank_to_update & 0x80 ? slot ^ 0x04 : slot;

	if (index & 0x04) {
		return (m195.mmc3[index - 2]);
	}
	return ((m195.mmc3[index >> 1] & ~1) | (index & 0x01));
}
