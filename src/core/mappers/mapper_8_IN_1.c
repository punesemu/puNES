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

INLINE static void prg_fix_8_IN_1(BYTE value);
INLINE static void prg_swap_8_IN_1(WORD address, WORD value);
INLINE static void chr_fix_8_IN_1(BYTE value);
INLINE static void chr_swap_8_IN_1(WORD address, WORD value);

struct _m8in1 {
	BYTE reg;
	BYTE mmc3[8];
} m8in1;

void map_init_8_IN_1(void) {
	EXTCL_AFTER_MAPPER_INIT(8_IN_1);
	EXTCL_CPU_WR_MEM(8_IN_1);
	EXTCL_SAVE_MAPPER(8_IN_1);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m8in1;
	mapper.internal_struct_size[0] = sizeof(m8in1);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m8in1, 0x00, sizeof(m8in1));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	m8in1.mmc3[0] = 0;
	m8in1.mmc3[1] = 2;
	m8in1.mmc3[2] = 4;
	m8in1.mmc3[3] = 5;
	m8in1.mmc3[4] = 6;
	m8in1.mmc3[5] = 7;
	m8in1.mmc3[6] = 0;
	m8in1.mmc3[7] = 0;

	if (mapper.write_vram && !info.chr.rom.banks_8k) {
		info.chr.rom.banks_8k = 32;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_8_IN_1(void) {
	prg_fix_8_IN_1(mmc3.bank_to_update);
	chr_fix_8_IN_1(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_8_IN_1(WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x9000:
		case 0x9001:
		case 0xB000:
		case 0xB001:
		case 0xD000:
		case 0xD001:
		case 0xF000:
		case 0xF001:
			m8in1.reg = value;
			prg_fix_8_IN_1(mmc3.bank_to_update);
			chr_fix_8_IN_1(mmc3.bank_to_update);
			return;
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				prg_fix_8_IN_1(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				chr_fix_8_IN_1(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			m8in1.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					chr_swap_8_IN_1(cbase ^ 0x0000, value & (~1));
					chr_swap_8_IN_1(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					chr_swap_8_IN_1(cbase ^ 0x0800, value & (~1));
					chr_swap_8_IN_1(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					chr_swap_8_IN_1(cbase ^ 0x1000, value);
					return;
				case 3:
					chr_swap_8_IN_1(cbase ^ 0x1400, value);
					return;
				case 4:
					chr_swap_8_IN_1(cbase ^ 0x1800, value);
					return;
				case 5:
					chr_swap_8_IN_1(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_8_IN_1(0xC000, value);
					} else {
						prg_swap_8_IN_1(0x8000, value);
					}
					return;
				case 7:
					prg_swap_8_IN_1(0xA000, value);
					return;
			}
			return;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_8_IN_1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m8in1.reg);
	save_slot_ele(mode, slot, m8in1.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_8_IN_1(BYTE value) {
	if (value & 0x40) {
		prg_swap_8_IN_1(0x8000, ~1);
		prg_swap_8_IN_1(0xC000, m8in1.mmc3[6]);
	} else {
		prg_swap_8_IN_1(0x8000, m8in1.mmc3[6]);
		prg_swap_8_IN_1(0xC000, ~1);
	}
	prg_swap_8_IN_1(0xA000, m8in1.mmc3[7]);
	prg_swap_8_IN_1(0xE000, ~0);
}
INLINE static void prg_swap_8_IN_1(WORD address, WORD value) {
	if (m8in1.reg & 0x10) {
		value = ((m8in1.reg & 0x0C) << 2) | (value & 0x0F);
	} else {
		value = ((m8in1.reg & 0x0F) << 2) | ((address >> 13) & 0x03);
	}
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_8_IN_1(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_8_IN_1(cbase ^ 0x0000, m8in1.mmc3[0] & (~1));
	chr_swap_8_IN_1(cbase ^ 0x0400, m8in1.mmc3[0] |   1);
	chr_swap_8_IN_1(cbase ^ 0x0800, m8in1.mmc3[1] & (~1));
	chr_swap_8_IN_1(cbase ^ 0x0C00, m8in1.mmc3[1] |   1);
	chr_swap_8_IN_1(cbase ^ 0x1000, m8in1.mmc3[2]);
	chr_swap_8_IN_1(cbase ^ 0x1400, m8in1.mmc3[3]);
	chr_swap_8_IN_1(cbase ^ 0x1800, m8in1.mmc3[4]);
	chr_swap_8_IN_1(cbase ^ 0x1C00, m8in1.mmc3[5]);
}
INLINE static void chr_swap_8_IN_1(WORD address, WORD value) {
	value = ((m8in1.reg & 0x0C) << 5) | (value & 0x7F);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
