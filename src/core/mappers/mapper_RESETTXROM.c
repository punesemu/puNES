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
#include <limits.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_RESETTXROM(BYTE value);
INLINE static void prg_swap_RESETTXROM(WORD address, WORD value);
INLINE static void chr_fix_RESETTXROM(BYTE value);
INLINE static void chr_swap_RESETTXROM(WORD address, WORD value);

struct _resettxrom {
	BYTE reg;
	WORD mmc3[8];

	// da non salvare
	struct _resettxrom_prg {
		WORD outer[2];
		WORD mask[2];
	} prg;
	struct _resettxrom_chr {
		WORD outer;
		WORD mask;
	} chr;
} resettxrom;

void map_init_RESETTXROM(void) {
	EXTCL_AFTER_MAPPER_INIT(RESETTXROM);
	EXTCL_CPU_WR_MEM(RESETTXROM);
	EXTCL_SAVE_MAPPER(RESETTXROM);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&resettxrom;
	mapper.internal_struct_size[0] = sizeof(resettxrom);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	if (info.reset == RESET) {
		resettxrom.reg = (resettxrom.reg + 1) & 0x03;
	} else if (info.reset >= HARD) {
		memset(&resettxrom, 0x00, sizeof(resettxrom));
	}

	resettxrom.mmc3[0] = 0;
	resettxrom.mmc3[1] = 2;
	resettxrom.mmc3[2] = 4;
	resettxrom.mmc3[3] = 5;
	resettxrom.mmc3[4] = 6;
	resettxrom.mmc3[5] = 7;
	resettxrom.mmc3[6] = 0;
	resettxrom.mmc3[7] = 0;

	switch (info.mapper.submapper) {
		default:
		case 0:
			resettxrom.prg.outer[0] = resettxrom.prg.outer[1] = 4;
			resettxrom.prg.mask[0] = resettxrom.prg.mask[1] = 0x0F;
			resettxrom.chr.outer = 7;
			resettxrom.chr.mask = 0x7F;
			break;
		case 1:
			resettxrom.prg.outer[0] = resettxrom.prg.outer[1] = 5;
			resettxrom.prg.mask[0] = resettxrom.prg.mask[1] = 0x1F;
			resettxrom.chr.outer = 7;
			resettxrom.chr.mask = 0x7F;
			break;
		case 2:
			resettxrom.prg.outer[0] = resettxrom.prg.outer[1] = 4;
			resettxrom.prg.mask[0] = resettxrom.prg.mask[1] = 0x0F;
			resettxrom.chr.outer = 8;
			resettxrom.chr.mask = 0xFF;
			break;
		case 3:
			resettxrom.prg.outer[0] = resettxrom.prg.outer[1] = 5;
			resettxrom.prg.mask[0] = resettxrom.prg.mask[1] = 0x1F;
			resettxrom.chr.outer = 8;
			resettxrom.chr.mask = 0xFF;
			break;
		case 4:
			resettxrom.prg.outer[0] = 5;
			resettxrom.prg.mask[0] = 0x1F;
			resettxrom.prg.outer[1] = 4;
			resettxrom.prg.mask[1] = 0x0F;
			resettxrom.chr.outer = 7;
			resettxrom.chr.mask = 0x7F;
			break;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_RESETTXROM(void) {
	prg_fix_RESETTXROM(mmc3.bank_to_update);
	chr_fix_RESETTXROM(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_RESETTXROM(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				prg_fix_RESETTXROM(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				chr_fix_RESETTXROM(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			resettxrom.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					chr_swap_RESETTXROM(cbase ^ 0x0000, value & (~1));
					chr_swap_RESETTXROM(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					chr_swap_RESETTXROM(cbase ^ 0x0800, value & (~1));
					chr_swap_RESETTXROM(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					chr_swap_RESETTXROM(cbase ^ 0x1000, value);
					return;
				case 3:
					chr_swap_RESETTXROM(cbase ^ 0x1400, value);
					return;
				case 4:
					chr_swap_RESETTXROM(cbase ^ 0x1800, value);
					return;
				case 5:
					chr_swap_RESETTXROM(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_RESETTXROM(0xC000, value);
					} else {
						prg_swap_RESETTXROM(0x8000, value);
					}
					return;
				case 7:
					prg_swap_RESETTXROM(0xA000, value);
					return;
			}
			return;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_RESETTXROM(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, resettxrom.reg);
	save_slot_ele(mode, slot, resettxrom.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_RESETTXROM(BYTE value) {
	if (value & 0x40) {
		prg_swap_RESETTXROM(0x8000, ~1);
		prg_swap_RESETTXROM(0xC000, resettxrom.mmc3[6]);
	} else {
		prg_swap_RESETTXROM(0x8000, resettxrom.mmc3[6]);
		prg_swap_RESETTXROM(0xC000, ~1);
	}
	prg_swap_RESETTXROM(0xA000, resettxrom.mmc3[7]);
	prg_swap_RESETTXROM(0xE000, ~0);
}
INLINE static void prg_swap_RESETTXROM(WORD address, WORD value) {
	WORD base = resettxrom.reg << ((resettxrom.reg == 0) ? resettxrom.prg.outer[0] : resettxrom.prg.outer[1]);
	WORD mask = (resettxrom.reg == 0) ? resettxrom.prg.mask[0] : resettxrom.prg.mask[1];

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_RESETTXROM(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_RESETTXROM(cbase ^ 0x0000, resettxrom.mmc3[0] & (~1));
	chr_swap_RESETTXROM(cbase ^ 0x0400, resettxrom.mmc3[0] |   1);
	chr_swap_RESETTXROM(cbase ^ 0x0800, resettxrom.mmc3[1] & (~1));
	chr_swap_RESETTXROM(cbase ^ 0x0C00, resettxrom.mmc3[1] |   1);
	chr_swap_RESETTXROM(cbase ^ 0x1000, resettxrom.mmc3[2]);
	chr_swap_RESETTXROM(cbase ^ 0x1400, resettxrom.mmc3[3]);
	chr_swap_RESETTXROM(cbase ^ 0x1800, resettxrom.mmc3[4]);
	chr_swap_RESETTXROM(cbase ^ 0x1C00, resettxrom.mmc3[5]);
}
INLINE static void chr_swap_RESETTXROM(WORD address, WORD value) {
	WORD base = resettxrom.reg << resettxrom.chr.outer;
	WORD mask = resettxrom.chr.mask;

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
