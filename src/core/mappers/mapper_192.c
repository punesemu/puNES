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

INLINE static void prg_fix_192(BYTE value);
INLINE static void prg_swap_192(WORD address, WORD value);
INLINE static void chr_fix_192(BYTE value);
INLINE static void chr_swap_192(WORD address, WORD value);

struct _m192 {
	WORD mmc3[8];
} m192;

void map_init_192(void) {
	EXTCL_AFTER_MAPPER_INIT(192);
	EXTCL_CPU_WR_MEM(192);
	EXTCL_SAVE_MAPPER(192);
	EXTCL_WR_CHR(192);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m192;
	mapper.internal_struct_size[0] = sizeof(m192);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m192, 0x00, sizeof(m192));

	m192.mmc3[0] = 0;
	m192.mmc3[1] = 2;
	m192.mmc3[2] = 4;
	m192.mmc3[3] = 5;
	m192.mmc3[4] = 6;
	m192.mmc3[5] = 7;
	m192.mmc3[6] = 0;
	m192.mmc3[7] = 0;

	map_chr_ram_extra_init(0x1000);

	// You Ling Xing Dong (Asia) (Unl).nes
	if (info.crc32.prg == 0xFF93EFF0) {
		mirroring_V();
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_192(void) {
	prg_fix_192(mmc3.bank_to_update);
	chr_fix_192(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_192(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				prg_fix_192(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				chr_fix_192(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			m192.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					chr_swap_192(cbase ^ 0x0000, value & (~1));
					chr_swap_192(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					chr_swap_192(cbase ^ 0x0800, value & (~1));
					chr_swap_192(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					chr_swap_192(cbase ^ 0x1000, value);
					return;
				case 3:
					chr_swap_192(cbase ^ 0x1400, value);
					return;
				case 4:
					chr_swap_192(cbase ^ 0x1800, value);
					return;
				case 5:
					chr_swap_192(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_192(0xC000, value);
					} else {
						prg_swap_192(0x8000, value);
					}
					return;
				case 7:
					prg_swap_192(0xA000, value);
					return;
			}
			return;
		}
		case 0xA001:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_192(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m192.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		chr_fix_192(mmc3.bank_to_update);
	}

	return (EXIT_OK);
}
void extcl_wr_chr_192(WORD address, BYTE value) {
	BYTE slot = address >> 10;

	if ((chr.bank_1k[slot] >= chr.extra.data) && (chr.bank_1k[slot] < (chr.extra.data + chr.extra.size))) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_192(BYTE value) {
	if (value & 0x40) {
		prg_swap_192(0x8000, ~1);
		prg_swap_192(0xC000, m192.mmc3[6]);
	} else {
		prg_swap_192(0x8000, m192.mmc3[6]);
		prg_swap_192(0xC000, ~1);
	}
	prg_swap_192(0xA000, m192.mmc3[7]);
	prg_swap_192(0xE000, ~0);
}
INLINE static void prg_swap_192(WORD address, WORD value) {
	value &= 0x3F;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_192(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_192(cbase ^ 0x0000, m192.mmc3[0] & (~1));
	chr_swap_192(cbase ^ 0x0400, m192.mmc3[0] |   1);
	chr_swap_192(cbase ^ 0x0800, m192.mmc3[1] & (~1));
	chr_swap_192(cbase ^ 0x0C00, m192.mmc3[1] |   1);
	chr_swap_192(cbase ^ 0x1000, m192.mmc3[2]);
	chr_swap_192(cbase ^ 0x1400, m192.mmc3[3]);
	chr_swap_192(cbase ^ 0x1800, m192.mmc3[4]);
	chr_swap_192(cbase ^ 0x1C00, m192.mmc3[5]);
}
INLINE static void chr_swap_192(WORD address, WORD value) {
	BYTE slot = address >> 10;

	if ((value & 0xF8) == 0x08) {
		value &= 0x03;
		chr.bank_1k[slot] = &chr.extra.data[value << 10];
	} else {
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[slot] = chr_pnt(value << 10);
	}
}
