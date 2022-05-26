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

INLINE static void prg_fix_126(BYTE value);
INLINE static void prg_swap_126(WORD address, WORD value);
INLINE static void chr_fix_126(BYTE value);
INLINE static void chr_swap_126(WORD address, WORD value);

struct _m126 {
	BYTE reg[4];
	WORD mmc3[8];
} m126;
struct _m126tmp {
	BYTE dipswitch;
} m126tmp;

void map_init_126(void) {
	EXTCL_AFTER_MAPPER_INIT(126);
	EXTCL_CPU_WR_MEM(126);
	EXTCL_CPU_RD_MEM(126);
	EXTCL_SAVE_MAPPER(126);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m126;
	mapper.internal_struct_size[0] = sizeof(m126);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m126, 0x00, sizeof(m126));

	m126.mmc3[0] = 0;
	m126.mmc3[1] = 2;
	m126.mmc3[2] = 4;
	m126.mmc3[3] = 5;
	m126.mmc3[4] = 6;
	m126.mmc3[5] = 7;
	m126.mmc3[6] = 0;
	m126.mmc3[7] = 0;

	if (info.reset == RESET) {
		m126tmp.dipswitch = (m126tmp.dipswitch + 1) & 0x03;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m126tmp.dipswitch = 0;
	}

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_126(void) {
	prg_fix_126(mmc3.bank_to_update);
	chr_fix_126(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_126(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		BYTE reg = address & 0x03;

		if (!(m126.reg[3] & 0x80)) {
			m126.reg[reg] = value;
			prg_fix_126(mmc3.bank_to_update);
			chr_fix_126(mmc3.bank_to_update);
		} else if (reg == 2) {
			m126.reg[2] = (m126.reg[2] & 0xFC) | (value & 0x03);
			chr_fix_126(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
					prg_fix_126(value);
				}
				if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
					chr_fix_126(value);
				}
				mmc3.bank_to_update = value;
				return;
			case 0x8001: {
				WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

				m126.mmc3[mmc3.bank_to_update & 0x07] = value;

				switch (mmc3.bank_to_update & 0x07) {
					case 0:
						chr_swap_126(cbase ^ 0x0000, value & (~1));
						chr_swap_126(cbase ^ 0x0400, value | 1);
						return;
					case 1:
						chr_swap_126(cbase ^ 0x0800, value & (~1));
						chr_swap_126(cbase ^ 0x0C00, value | 1);
						return;
					case 2:
						chr_swap_126(cbase ^ 0x1000, value);
						return;
					case 3:
						chr_swap_126(cbase ^ 0x1400, value);
						return;
					case 4:
						chr_swap_126(cbase ^ 0x1800, value);
						return;
					case 5:
						chr_swap_126(cbase ^ 0x1C00, value);
						return;
					case 6:
						if (mmc3.bank_to_update & 0x40) {
							prg_swap_126(0xC000, value);
						} else {
							prg_swap_126(0x8000, value);
						}
						return;
					case 7:
						prg_swap_126(0xA000, value);
						return;
				}
				return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_126(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (m126.reg[1] & 0x01)) {
		return ((openbus & 0xFC) | m126tmp.dipswitch);
	}
	return (openbus);
}
BYTE extcl_save_mapper_126(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m126.reg);
	save_slot_ele(mode, slot, m126.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_126(BYTE value) {
	if (value & 0x40) {
		prg_swap_126(0x8000, ~1);
		prg_swap_126(0xC000, m126.mmc3[6]);
	} else {
		prg_swap_126(0x8000, m126.mmc3[6]);
		prg_swap_126(0xC000, ~1);
	}
	prg_swap_126(0xA000, m126.mmc3[7]);
	prg_swap_126(0xE000, ~0);
}
INLINE static void prg_swap_126(WORD address, WORD value) {
	WORD base = ((m126.reg[0] & (0x06 | ((m126.reg[0] & 0x40) >> 6))) << 4) | ((m126.reg[0] & 0x10) << 3);
	WORD mask = ((~m126.reg[0] >> 2) & 0x10) | 0x0F;
	BYTE mode = m126.reg[3] & 0x03;
	BYTE bank = (address >> 13) & 0x03;

	if (mode) {
		mask = mode == 0x03 ? 0xFC : 0xFE;
		value = (base | (m126.mmc3[6] & mask)) | (bank & ~mask);
	} else {
		value = base | (value & mask);
	}

	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, bank, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_126(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_126(cbase ^ 0x0000, m126.mmc3[0] & (~1));
	chr_swap_126(cbase ^ 0x0400, m126.mmc3[0] |   1);
	chr_swap_126(cbase ^ 0x0800, m126.mmc3[1] & (~1));
	chr_swap_126(cbase ^ 0x0C00, m126.mmc3[1] |   1);
	chr_swap_126(cbase ^ 0x1000, m126.mmc3[2]);
	chr_swap_126(cbase ^ 0x1400, m126.mmc3[3]);
	chr_swap_126(cbase ^ 0x1800, m126.mmc3[4]);
	chr_swap_126(cbase ^ 0x1C00, m126.mmc3[5]);
}
INLINE static void chr_swap_126(WORD address, WORD value) {
	WORD base = ((~m126.reg[0] & 0x0080) & m126.reg[2]) | (((m126.reg[0] << 4) & 0x0080) & m126.reg[0])
		| ((m126.reg[0] << 3) & 0x0100) | ((m126.reg[0] << 5) & 0x0200);
	WORD mask = 0x7F;

	if (!(m126.reg[3] & 0x10)) {
		mask = (m126.reg[0] & 0x80) - 1;
	} else {
		value = ((m126.reg[2] & 0x0F) << 3) | (address >> 10);
	}

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
