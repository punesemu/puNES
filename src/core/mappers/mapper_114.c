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

void prg_swap_mmc3_114(WORD address, WORD value);
void chr_swap_mmc3_114(WORD address, WORD value);

INLINE static void tmp_fix_114(BYTE max, BYTE index, const WORD *ds);

_m114 m114;
struct _m114tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m114tmp;

void map_init_114(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(114);
	EXTCL_CPU_RD_MEM(114);
	EXTCL_SAVE_MAPPER(114);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m114;
	mapper.internal_struct_size[0] = sizeof(m114);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m114, 0x00, sizeof(m114));

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_114;
	MMC3_chr_swap = chr_swap_mmc3_114;

	if (info.reset == RESET) {
		if (m114tmp.ds_used) {
			m114tmp.index = (m114tmp.index + 1) % m114tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m114tmp, 0x00, sizeof(m114tmp));

		{
			static WORD ds[] = { 0x00 };

			tmp_fix_114(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_114(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m114.reg[1] & 0x01) && memmap_adr_is_writable(MMCPU(address))) {
			m114.reg[address & 0x03] = value;
			MMC3_chr_fix();
			MMC3_prg_fix();
			return;
		}
	}
	if (address >= 0x8000) {
		static WORD m114_mmc3_adr[2][8] = {
			{ 0xA001, 0xA000, 0x8000, 0xC000, 0x8001, 0xC001, 0xE000, 0xE001 },
			{ 0xA001, 0x8001, 0x8000, 0xC001, 0xA000, 0xC000, 0xE000, 0xE001 }
		};
		static BYTE m114_r8000_idx[2][8] = {
			{ 0, 3, 1, 5, 6, 7, 2, 4 },
			{ 0, 2, 5, 3, 6, 1, 7, 4 },
		};
		WORD mmc_address = m114_mmc3_adr[info.mapper.submapper][((address & 0x6000) >> 12) | (address & 0x01)];

		extcl_cpu_wr_mem_MMC3(mmc_address, value);
		if (mmc_address == 0x8000) {
			mmc3.bank_to_update = (value & 0xF8) | m114_r8000_idx[info.mapper.submapper][value & 0x07];
		}
	}
}
BYTE extcl_cpu_rd_mem_114(WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return ((address & 0x03) == 2
			? (m114tmp.dipswitch[m114tmp.index] & 0x07) | (openbus & 0xF8)
			: openbus);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_114(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m114.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_114(WORD address, WORD value) {
	if (m114.reg[0] & 0x80) {
		value = ((m114.reg[0] & 0x0E) | (m114.reg[0] & 0x20 ? (address & 0x4000) >> 14 : m114.reg[0] & 0x01)) << 1;
		value |= (address & 0x2000) >> 13;
	}
	prg_swap_MMC3_base(address, value);
}
void chr_swap_mmc3_114(WORD address, WORD value) {
	WORD base = (m114.reg[1] & 0x01) << 8;
	WORD mask = 0xFF;

	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}

INLINE static void tmp_fix_114(BYTE max, BYTE index, const WORD *ds) {
	m114tmp.ds_used = TRUE;
	m114tmp.max = max;
	m114tmp.index = index;
	m114tmp.dipswitch = ds;
}
