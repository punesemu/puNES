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

INLINE static void prg_fix_432(BYTE value);
INLINE static void prg_swap_432(WORD address, WORD value);
INLINE static void chr_fix_432(BYTE value);
INLINE static void chr_swap_432(WORD address, WORD value);

static const BYTE dipswitch_432[4] = { 0, 1, 2, 3 };
static const SBYTE dipswitch_index_432[][4] = {
	{ 0,  1,  2,  3 }, // 0
	{ 1,  0,  2,  3 }, // 1
};

struct _m432 {
	BYTE reg[2];
	WORD mmc3[8];
} m432;
struct _m432tmp {
	BYTE select;
	BYTE index;
	BYTE dipswitch;
} m432tmp;

void map_init_432(void) {
	EXTCL_AFTER_MAPPER_INIT(432);
	EXTCL_CPU_WR_MEM(432);
	EXTCL_CPU_RD_MEM(432);
	EXTCL_SAVE_MAPPER(432);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m432;
	mapper.internal_struct_size[0] = sizeof(m432);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m432, 0x00, sizeof(m432));

	m432.mmc3[0] = 0;
	m432.mmc3[1] = 2;
	m432.mmc3[2] = 4;
	m432.mmc3[3] = 5;
	m432.mmc3[4] = 6;
	m432.mmc3[5] = 7;
	m432.mmc3[6] = 0;
	m432.mmc3[7] = 0;

	if (info.reset == RESET) {
		do {
			m432tmp.index = (m432tmp.index + 1) & 0x03;
		} while (dipswitch_index_432[m432tmp.select][m432tmp.index] < 0);
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0xE736A4BE) { // 160000000-in-1.nes
			m432tmp.select = 1;
			m432tmp.index = 1;
		} else {
			m432tmp.select = 0;
			m432tmp.index = 0;
		}
	}
	m432tmp.dipswitch = dipswitch_432[dipswitch_index_432[m432tmp.select][m432tmp.index]];

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_432(void) {
	prg_fix_432(mmc3.bank_to_update);
	chr_fix_432(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_432(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			m432.reg[address & 0x01] = value;
			prg_fix_432(mmc3.bank_to_update);
			chr_fix_432(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
					prg_fix_432(value);
				}
				if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
					chr_fix_432(value);
				}
				mmc3.bank_to_update = value;
				return;
			case 0x8001: {
				WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

				m432.mmc3[mmc3.bank_to_update & 0x07] = value;

				switch (mmc3.bank_to_update & 0x07) {
					case 0:
						chr_swap_432(cbase ^ 0x0000, value & (~1));
						chr_swap_432(cbase ^ 0x0400, value | 1);
						return;
					case 1:
						chr_swap_432(cbase ^ 0x0800, value & (~1));
						chr_swap_432(cbase ^ 0x0C00, value | 1);
						return;
					case 2:
						chr_swap_432(cbase ^ 0x1000, value);
						return;
					case 3:
						chr_swap_432(cbase ^ 0x1400, value);
						return;
					case 4:
						chr_swap_432(cbase ^ 0x1800, value);
						return;
					case 5:
						chr_swap_432(cbase ^ 0x1C00, value);
						return;
					case 6:
						if (mmc3.bank_to_update & 0x40) {
							prg_swap_432(0xC000, value);
						} else {
							prg_swap_432(0x8000, value);
						}
						return;
					case 7:
						prg_swap_432(0xA000, value);
						return;
				}
				return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_432(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address > 0x8000) && (m432.reg[0] & 0x01)) {
		return (m432tmp.dipswitch);
	}
	return (openbus);
}
BYTE extcl_save_mapper_432(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m432.reg);
	save_slot_ele(mode, slot, m432.mmc3);
	save_slot_ele(mode, slot, m432tmp.index);
	save_slot_ele(mode, slot, m432tmp.dipswitch);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_432(BYTE value) {
	if (value & 0x40) {
		prg_swap_432(0x8000, ~1);
		prg_swap_432(0xC000, m432.mmc3[6]);
	} else {
		prg_swap_432(0x8000, m432.mmc3[6]);
		prg_swap_432(0xC000, ~1);
	}
	prg_swap_432(0xA000, m432.mmc3[7]);
	prg_swap_432(0xE000, ~0);
}
INLINE static void prg_swap_432(WORD address, WORD value) {
	WORD base = ((m432.reg[1] & 0x01) << 4) | ((m432.reg[1] & 0x10) << 1);
	WORD mask = 0x1F >> ((m432.reg[1] & 0x02) >> 1);
	BYTE bank = (address >> 13) & 0x03;

	if (m432.reg[1] & 0x40) {
		if (!(bank & 0x02)) {
			value = m432.mmc3[5 + (bank & 0x01)] & ~((m432.reg[1] & 0x80) >> 6);
		} else {
			value = m432.mmc3[5 + (bank & 0x01)] | ((m432.reg[1] & 0x80) >> 6);
		}
	}

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, bank, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_432(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_432(cbase ^ 0x0000, m432.mmc3[0] & (~1));
	chr_swap_432(cbase ^ 0x0400, m432.mmc3[0] |   1);
	chr_swap_432(cbase ^ 0x0800, m432.mmc3[1] & (~1));
	chr_swap_432(cbase ^ 0x0C00, m432.mmc3[1] |   1);
	chr_swap_432(cbase ^ 0x1000, m432.mmc3[2]);
	chr_swap_432(cbase ^ 0x1400, m432.mmc3[3]);
	chr_swap_432(cbase ^ 0x1800, m432.mmc3[4]);
	chr_swap_432(cbase ^ 0x1C00, m432.mmc3[5]);
}
INLINE static void chr_swap_432(WORD address, WORD value) {
	WORD base = ((m432.reg[1] & 0x08) << 5) | ((m432.reg[1] & 0x01) << 7);
	WORD mask = 0xFF >> ((m432.reg[1] & 0x04) >> 2);

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
