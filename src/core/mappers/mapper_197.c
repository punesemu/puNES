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

INLINE static void prg_fix_197(BYTE value);
INLINE static void prg_swap_197(WORD address, WORD value);
INLINE static void chr_fix_197(void);

struct _m197 {
	BYTE reg;
	WORD mmc3[8];
} m197;

void map_init_197(void) {
	EXTCL_AFTER_MAPPER_INIT(197);
	EXTCL_CPU_WR_MEM(197);
	EXTCL_SAVE_MAPPER(197);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m197;
	mapper.internal_struct_size[0] = sizeof(m197);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m197, 0x00, sizeof(m197));

	m197.mmc3[0] = 0;
	m197.mmc3[1] = 2;
	m197.mmc3[2] = 4;
	m197.mmc3[3] = 5;
	m197.mmc3[4] = 6;
	m197.mmc3[5] = 7;
	m197.mmc3[6] = 0;
	m197.mmc3[7] = 0;

	if (info.mapper.submapper == 3) {
		info.mapper.extend_wr = TRUE;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_197(void) {
	prg_fix_197(mmc3.bank_to_update);
	chr_fix_197();
}
void extcl_cpu_wr_mem_197(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			m197.reg = value;
			prg_fix_197(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
					prg_fix_197(value);
				}
				if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
					chr_fix_197();
				}
				mmc3.bank_to_update = value;
				return;
			case 0x8001:
				m197.mmc3[mmc3.bank_to_update & 0x07] = value;

				switch (mmc3.bank_to_update & 0x07) {
					case 0:
						chr_fix_197();
						return;
					case 1:
						chr_fix_197();
						return;
					case 2:
						chr_fix_197();
						return;
					case 3:
						chr_fix_197();
						return;
					case 4:
						chr_fix_197();
						return;
					case 5:
						chr_fix_197();
						return;
					case 6:
						if (mmc3.bank_to_update & 0x40) {
							prg_swap_197(0xC000, value);
						} else {
							prg_swap_197(0x8000, value);
						}
						return;
					case 7:
						prg_swap_197(0xA000, value);
						return;
				}
				return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_197(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m197.reg);
	save_slot_ele(mode, slot, m197.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_197(BYTE value) {
	if (value & 0x40) {
		prg_swap_197(0x8000, ~1);
		prg_swap_197(0xC000, m197.mmc3[6]);
	} else {
		prg_swap_197(0x8000, m197.mmc3[6]);
		prg_swap_197(0xC000, ~1);
	}
	prg_swap_197(0xA000, m197.mmc3[7]);
	prg_swap_197(0xE000, ~0);
}
INLINE static void prg_swap_197(WORD address, WORD value) {
	WORD base = info.mapper.submapper == 3 ? (m197.reg & 0x01) << 4 : 0;
	WORD mask = info.mapper.submapper == 3 ? 0x1F >> ((m197.reg & 0x08) >> 3) : 0x3F;

	value = base | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_197(void) {
	WORD slot[4];
	DBWORD bank;

	switch (info.mapper.submapper) {
		default:
		case 0:
		case 3:
			slot[0] = m197.mmc3[0] & (~1);
			slot[1] = m197.mmc3[0] | 1;
			slot[2] = m197.mmc3[2];
			slot[3] = m197.mmc3[3];
			break;
		case 1:
			slot[0] = m197.mmc3[1] & (~1);
			slot[1] = m197.mmc3[1] | 1;
			slot[2] = m197.mmc3[4];
			slot[3] = m197.mmc3[5];
			break;
		case 2:
			slot[0] = m197.mmc3[0] & (~1);
			slot[1] = m197.mmc3[1] | 1;
			slot[2] = m197.mmc3[2];
			slot[3] = m197.mmc3[5];
			break;
	}

	bank = slot[0];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);

	bank = slot[1];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[2] = chr_pnt(bank);
	chr.bank_1k[3] = chr_pnt(bank | 0x0400);

	bank = slot[2];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[4] = chr_pnt(bank);
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);

	bank = slot[3];
	_control_bank(bank, info.chr.rom.max.banks_2k)
	bank <<= 11;
	chr.bank_1k[6] = chr_pnt(bank);
	chr.bank_1k[7] = chr_pnt(bank | 0x0400);
}
