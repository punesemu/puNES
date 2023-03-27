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

INLINE static void prg_fix_334(BYTE value);
INLINE static void prg_swap_334(WORD address, WORD value);
INLINE static void chr_fix_334(BYTE value);
INLINE static void chr_swap_334(WORD address, WORD value);
INLINE static void tmp_fix_334(BYTE max, BYTE index, const BYTE *ds);

struct _m334 {
	BYTE reg;
	WORD mmc3[8];
} m334;
struct _m334tmp {
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m334tmp;

void map_init_334(void) {
	EXTCL_AFTER_MAPPER_INIT(334);
	EXTCL_CPU_WR_MEM(334);
	EXTCL_CPU_RD_MEM(334);
	EXTCL_SAVE_MAPPER(334);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m334;
	mapper.internal_struct_size[0] = sizeof(m334);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m334, 0x00, sizeof(m334));

	m334.mmc3[0] = 0;
	m334.mmc3[1] = 2;
	m334.mmc3[2] = 4;
	m334.mmc3[3] = 5;
	m334.mmc3[4] = 6;
	m334.mmc3[5] = 7;
	m334.mmc3[6] = 0;
	m334.mmc3[7] = 0;

	if (info.reset == RESET) {
		if (m334tmp.dipswitch) {
			m334tmp.index = (m334tmp.index + 1) % m334tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&m334tmp, 0x00, sizeof(m334tmp));
		if (info.crc32.prg == 0x7EC6DF24) { // 5-in-1 (Multi)[Unknown][1993 Copyrights].nes
			static const BYTE ds[] = { 0x01, 0x00 };

			tmp_fix_334(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_334(void) {
	prg_fix_334(mmc3.bank_to_update);
	chr_fix_334(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_334(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active & !(address & 0x0001)) {
			m334.reg = value;
			prg_fix_334(mmc3.bank_to_update);
			chr_fix_334(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
					prg_fix_334(value);
				}
				if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
					chr_fix_334(value);
				}
				mmc3.bank_to_update = value;
				return;
			case 0x8001: {
				WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

				m334.mmc3[mmc3.bank_to_update & 0x07] = value;

				switch (mmc3.bank_to_update & 0x07) {
					case 0:
						chr_swap_334(cbase ^ 0x0000, value & (~1));
						chr_swap_334(cbase ^ 0x0400, value | 1);
						return;
					case 1:
						chr_swap_334(cbase ^ 0x0800, value & (~1));
						chr_swap_334(cbase ^ 0x0C00, value | 1);
						return;
					case 2:
						chr_swap_334(cbase ^ 0x1000, value);
						return;
					case 3:
						chr_swap_334(cbase ^ 0x1400, value);
						return;
					case 4:
						chr_swap_334(cbase ^ 0x1800, value);
						return;
					case 5:
						chr_swap_334(cbase ^ 0x1C00, value);
						return;
					case 6:
						if (mmc3.bank_to_update & 0x40) {
							prg_swap_334(0xC000, value);
						} else {
							prg_swap_334(0x8000, value);
						}
						return;
					case 7:
						prg_swap_334(0xA000, value);
						return;
				}
				return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_334(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_rd_active && (address & 0x0002)) {
			return ((openbus & 0xFE) | (m334tmp.dipswitch[m334tmp.index] & 0x01));
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_334(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m334.reg);
	save_slot_ele(mode, slot, m334.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_334(UNUSED(BYTE value)) {
	WORD bank = m334.reg >> 1;

	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void prg_swap_334(UNUSED(WORD address), UNUSED(WORD value)) {}
INLINE static void chr_fix_334(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_334(cbase ^ 0x0000, m334.mmc3[0] & (~1));
	chr_swap_334(cbase ^ 0x0400, m334.mmc3[0] |   1);
	chr_swap_334(cbase ^ 0x0800, m334.mmc3[1] & (~1));
	chr_swap_334(cbase ^ 0x0C00, m334.mmc3[1] |   1);
	chr_swap_334(cbase ^ 0x1000, m334.mmc3[2]);
	chr_swap_334(cbase ^ 0x1400, m334.mmc3[3]);
	chr_swap_334(cbase ^ 0x1800, m334.mmc3[4]);
	chr_swap_334(cbase ^ 0x1C00, m334.mmc3[5]);
}
INLINE static void chr_swap_334(WORD address, WORD value) {
	WORD base = 0;
	WORD mask = 0xFF;

	value = base | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
INLINE static void tmp_fix_334(BYTE max, BYTE index, const BYTE *ds) {
	m334tmp.max = max;
	m334tmp.index = index;
	m334tmp.dipswitch = ds;
}
