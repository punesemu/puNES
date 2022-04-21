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

INLINE static void prg_fix_Coolboy(BYTE value);
INLINE static void prg_swap_Coolboy(WORD address, WORD value);
INLINE static void chr_fix_Coolboy(BYTE value);
INLINE static void chr_swap_Coolboy(WORD address, WORD value);

struct _coolboy {
	BYTE reg[4];
	BYTE mmc3[8];

	BYTE model;
	WORD rstart;
	WORD rstop;
} coolboy;

void map_init_Coolboy(BYTE model) {
	EXTCL_AFTER_MAPPER_INIT(Coolboy);
	EXTCL_CPU_WR_MEM(Coolboy);
	EXTCL_SAVE_MAPPER(Coolboy);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&coolboy;
	mapper.internal_struct_size[0] = sizeof(coolboy);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&coolboy, 0x00, sizeof(coolboy));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	coolboy.model = model;

	if ((mapper.write_vram == TRUE) && !info.chr.rom[0].banks_8k) {
		info.chr.rom[0].banks_8k = 32;
	}

	if (model == COOLBOY) {
		coolboy.rstart = 0x6000;
		coolboy.rstop = 0x6FFF;
	} else {
		coolboy.rstart = 0x5000;
		coolboy.rstop = 0x5FFF;
	}

	coolboy.mmc3[0] = 0;
	coolboy.mmc3[1] = 2;
	coolboy.mmc3[2] = 4;
	coolboy.mmc3[3] = 5;
	coolboy.mmc3[4] = 6;
	coolboy.mmc3[5] = 7;
	coolboy.mmc3[6] = 0;
	coolboy.mmc3[7] = 1;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_Coolboy(void) {
	// posso farlo solo dopo il map_prg_ram_init();
	prg_fix_Coolboy(mmc3.bank_to_update);
	chr_fix_Coolboy(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_Coolboy(WORD address, BYTE value) {
	if ((address >= coolboy.rstart) && (address <= coolboy.rstop)) {
		if ((coolboy.reg[3] & 0x90) != 0x80) {
			coolboy.reg[address & 0x03] = value;
			prg_fix_Coolboy(mmc3.bank_to_update);
			chr_fix_Coolboy(mmc3.bank_to_update);
		}
		return;
	}

	switch (address & 0xE001) {
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				prg_fix_Coolboy(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				chr_fix_Coolboy(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			coolboy.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					chr_swap_Coolboy(cbase ^ 0x0000, value & (~1));
					chr_swap_Coolboy(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					chr_swap_Coolboy(cbase ^ 0x0800, value & (~1));
					chr_swap_Coolboy(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					chr_swap_Coolboy(cbase ^ 0x1000, value);
					return;
				case 3:
					chr_swap_Coolboy(cbase ^ 0x1400, value);
					return;
				case 4:
					chr_swap_Coolboy(cbase ^ 0x1800, value);
					return;
				case 5:
					chr_swap_Coolboy(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_Coolboy(0xC000, value);
					} else {
						prg_swap_Coolboy(0x8000, value);
					}
					return;
				case 7:
					prg_swap_Coolboy(0xA000, value);
					return;
			}
			return;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_Coolboy(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, coolboy.reg);
	save_slot_ele(mode, slot, coolboy.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_Coolboy(BYTE value) {
	if (value & 0x40) {
		prg_swap_Coolboy(0x8000, ~1);
		prg_swap_Coolboy(0xC000, coolboy.mmc3[6]);
	} else {
		prg_swap_Coolboy(0x8000, coolboy.mmc3[6]);
		prg_swap_Coolboy(0xC000, ~1);
	}
	prg_swap_Coolboy(0xA000, coolboy.mmc3[7]);
	prg_swap_Coolboy(0xE000, ~0);
}
INLINE static void prg_swap_Coolboy(WORD address, WORD value) {
	uint32_t mask = ((0x3F | (coolboy.reg[1] & 0x40) | ((coolboy.reg[1] & 0x20) << 2)) ^ ((coolboy.reg[0] & 0x40) >> 2))
	    ^ ((coolboy.reg[1] & 0x80) >> 2);
	uint32_t base = ((coolboy.reg[0] & 0x07) >> 0) | ((coolboy.reg[1] & 0x10) >> 1) | ((coolboy.reg[1] & 0x0C) << 2)
	    | ((coolboy.reg[0] & 0x30) << 2);

	if ((coolboy.reg[3] & 0x40) && (value >= 0xFE) && !(mmc3.prg_rom_cfg != 0)) {
		switch (address & 0xE000) {
			case 0xC000:
			case 0xE000:
				value = 0;
				break;
		}
	}

	if (!(coolboy.reg[3] & 0x10)) {
		// modalita' MMC3
		value = (((base << 4) & ~mask)) | (value & mask);
	} else {
		// NROM mode
		BYTE emask;

		mask &= 0xF0;

		if ((((coolboy.reg[1] & 0x02) != 0))) {
			// 32kb mode
			emask = (coolboy.reg[3] & 0x0C) | ((address & 0x4000) >> 13);
		} else {
			// 16kb mode
			emask = coolboy.reg[3] & 0x0E;
		}
		value = ((base << 4) & ~mask) | (value & mask) | emask | ((address & 0x2000) >> 13);
	}

	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_Coolboy(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_Coolboy(cbase ^ 0x0000, coolboy.mmc3[0] & (~1));
	chr_swap_Coolboy(cbase ^ 0x0400, coolboy.mmc3[0] |   1);
	chr_swap_Coolboy(cbase ^ 0x0800, coolboy.mmc3[1] & (~1));
	chr_swap_Coolboy(cbase ^ 0x0C00, coolboy.mmc3[1] |   1);

	chr_swap_Coolboy(cbase ^ 0x1000, coolboy.mmc3[2]);
	chr_swap_Coolboy(cbase ^ 0x1400, coolboy.mmc3[3]);
	chr_swap_Coolboy(cbase ^ 0x1800, coolboy.mmc3[4]);
	chr_swap_Coolboy(cbase ^ 0x1c00, coolboy.mmc3[5]);
}
INLINE static void chr_swap_Coolboy(WORD address, WORD value) {
	DBWORD mask = 0xFF ^ (coolboy.reg[0] & 0x80);

	if (coolboy.reg[3] & 0x10) {
		if (coolboy.reg[3] & 0x40) {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			switch (cbase ^ address) {
				case 0x0400:
				case 0x0C00:
					value &= 0x7F;
					break;
			}
		}
		value =  (value & 0x80 & mask) | ((((coolboy.reg[0] & 0x08) << 4) & ~mask)) | ((coolboy.reg[2] & 0x0F) << 3)
			| ((address >> 10) & 0x07);
	} else {
		if (coolboy.reg[3] & 0x40) {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			switch (cbase ^ address) {
				case 0x0000:
					value = coolboy.mmc3[0];
					break;
				case 0x0800:
					value = coolboy.mmc3[1];
					break;
				case 0x0400:
				case 0x0C00:
					value = 0;
					break;
			}
		}
		value = (value & mask) | (((coolboy.reg[0] & 0x08) << 4) & ~mask);
	}

	control_bank(info.chr.rom[0].max.banks_1k)
	chr.bank_1k[address >> 10] = chr_chip_byte_pnt(0, value << 10);
}
