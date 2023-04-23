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

void prg_swap_Coolboy(WORD address, WORD value);
void chr_swap_Coolboy(WORD address, WORD value);

struct _coolboy {
	BYTE reg[4];
} coolboy;
struct _coolboytmp {
	BYTE model;
	WORD rstart;
	WORD rstop;
} coolboytmp;

void map_init_Coolboy(BYTE model) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
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

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&coolboy, 0x00, sizeof(coolboy));

	init_MMC3();
	MMC3_prg_swap = prg_swap_Coolboy;
	MMC3_chr_swap = chr_swap_Coolboy;

	coolboytmp.model = model;

	if (mapper.write_vram && !info.chr.rom.banks_8k) {
		info.chr.rom.banks_8k = 32;
	}

	if (model == COOLBOY) {
		coolboytmp.rstart = 0x6000;
		coolboytmp.rstop = 0x6FFF;
	} else {
		coolboytmp.rstart = 0x5000;
		coolboytmp.rstop = 0x5FFF;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_Coolboy(WORD address, BYTE value) {
	if ((address >= coolboytmp.rstart) && (address <= coolboytmp.rstop)) {
		if ((coolboy.reg[3] & 0x90) != 0x80) {
			coolboy.reg[address & 0x03] = value;
			MMC3_prg_fix(mmc3.bank_to_update);
			MMC3_chr_fix(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_Coolboy(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, coolboy.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_Coolboy(WORD address, WORD value) {
	uint32_t mask = ((0x3F | (coolboy.reg[1] & 0x40) | ((coolboy.reg[1] & 0x20) << 2)) ^ ((coolboy.reg[0] & 0x40) >> 2))
		^ ((coolboy.reg[1] & 0x80) >> 2);
	uint32_t base = ((coolboy.reg[0] & 0x07) >> 0) | ((coolboy.reg[1] & 0x10) >> 1) | ((coolboy.reg[1] & 0x0C) << 2)
		| ((coolboy.reg[0] & 0x30) << 2);

	if ((coolboy.reg[3] & 0x40) && (value >= 0xFE) && !((mmc3.bank_to_update & 0x40) != 0)) {
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
		BYTE emask = 0;

		mask &= 0xF0;
		if ((coolboy.reg[1] & 0x02) != 0) {
			// 32kb mode
			emask = (coolboy.reg[3] & 0x0C) | ((address & 0x4000) >> 13);
		} else {
			// 16kb mode
			emask = coolboy.reg[3] & 0x0E;
		}
		value = ((base << 4) & ~mask) | (value & mask) | emask | ((address & 0x2000) >> 13);
	}
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_Coolboy(WORD address, WORD value) {
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
		value = (value & 0x80 & mask) | ((((coolboy.reg[0] & 0x08) << 4) & ~mask)) | ((coolboy.reg[2] & 0x0F) << 3)
			| ((address >> 10) & 0x07);
	} else {
		if (coolboy.reg[3] & 0x40) {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			switch (cbase ^ address) {
				case 0x0000:
					value = mmc3.reg[0];
					break;
				case 0x0800:
					value = mmc3.reg[1];
					break;
				case 0x0400:
				case 0x0C00:
					value = 0;
					break;
			}
		}
		value = (value & mask) | (((coolboy.reg[0] & 0x08) << 4) & ~mask);
	}
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
