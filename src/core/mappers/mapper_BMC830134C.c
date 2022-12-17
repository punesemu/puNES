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

INLINE static void prg_fix_BMC830134C(BYTE value);
INLINE static void prg_swap_BMC830134C(WORD address, WORD value);
INLINE static void chr_fix_BMC830134C(BYTE value);
INLINE static void chr_swap_BMC830134C(WORD address, WORD value);

struct _bmc830134c {
	BYTE reg;
	WORD mmc3[8];
} bmc830134c;

void map_init_BMC830134C(void) {
	EXTCL_AFTER_MAPPER_INIT(BMC830134C);
	EXTCL_CPU_WR_MEM(BMC830134C);
	EXTCL_SAVE_MAPPER(BMC830134C);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&bmc830134c;
	mapper.internal_struct_size[0] = sizeof(bmc830134c);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&bmc830134c, 0x00, sizeof(bmc830134c));

	bmc830134c.mmc3[0] = 0;
	bmc830134c.mmc3[1] = 2;
	bmc830134c.mmc3[2] = 4;
	bmc830134c.mmc3[3] = 5;
	bmc830134c.mmc3[4] = 6;
	bmc830134c.mmc3[5] = 7;
	bmc830134c.mmc3[6] = 0;
	bmc830134c.mmc3[7] = 0;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_BMC830134C(void) {
	prg_fix_BMC830134C(mmc3.bank_to_update);
	chr_fix_BMC830134C(mmc3.bank_to_update);
}
void extcl_cpu_wr_mem_BMC830134C(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			bmc830134c.reg = value;
			prg_fix_BMC830134C(mmc3.bank_to_update);
			chr_fix_BMC830134C(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
					prg_fix_BMC830134C(value);
				}
				if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
					chr_fix_BMC830134C(value);
				}
				mmc3.bank_to_update = value;
				return;
			case 0x8001: {
				WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

				bmc830134c.mmc3[mmc3.bank_to_update & 0x07] = value;

				switch (mmc3.bank_to_update & 0x07) {
					case 0:
						chr_swap_BMC830134C(cbase ^ 0x0000, value & (~1));
						chr_swap_BMC830134C(cbase ^ 0x0400, value | 1);
						return;
					case 1:
						chr_swap_BMC830134C(cbase ^ 0x0800, value & (~1));
						chr_swap_BMC830134C(cbase ^ 0x0C00, value | 1);
						return;
					case 2:
						chr_swap_BMC830134C(cbase ^ 0x1000, value);
						return;
					case 3:
						chr_swap_BMC830134C(cbase ^ 0x1400, value);
						return;
					case 4:
						chr_swap_BMC830134C(cbase ^ 0x1800, value);
						return;
					case 5:
						chr_swap_BMC830134C(cbase ^ 0x1C00, value);
						return;
					case 6:
						if ((bmc830134c.reg & 0x06) == 0x06) {
							prg_fix_BMC830134C(mmc3.bank_to_update);
						} else {
							if (mmc3.bank_to_update & 0x40) {
								prg_swap_BMC830134C(0xC000, value);
							} else {
								prg_swap_BMC830134C(0x8000, value);
							}
						}
						return;
					case 7:
						if ((bmc830134c.reg & 0x06) == 0x06) {
							prg_fix_BMC830134C(mmc3.bank_to_update);
						} else {
							prg_swap_BMC830134C(0xA000, value);
						}
						return;
				}
				return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_BMC830134C(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc830134c.reg);
	save_slot_ele(mode, slot, bmc830134c.mmc3);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_BMC830134C(BYTE value) {
	if (value & 0x40) {
		prg_swap_BMC830134C(0x8000, ~1);
		prg_swap_BMC830134C(0xC000, bmc830134c.mmc3[6]);
	} else {
		prg_swap_BMC830134C(0x8000, bmc830134c.mmc3[6]);
		prg_swap_BMC830134C(0xC000, ~1);
	}
	prg_swap_BMC830134C(0xA000, bmc830134c.mmc3[7]);
	prg_swap_BMC830134C(0xE000, ~0);
}
INLINE static void prg_swap_BMC830134C(WORD address, WORD value) {
	WORD base = ((bmc830134c.reg >> 1) & 0x03) << 4;
	WORD mask = 0x0F;

	// GNROM mode
	if ((bmc830134c.reg & 0x06) == 0x06) {
		BYTE bank = (address >> 13) & 0x03;
		BYTE reg = bmc830134c.mmc3[0x06 | (bank & 0x01)];

		value = bank < 2 ? reg & 0xFD : reg | 0x02;
	}

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_BMC830134C(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_BMC830134C(cbase ^ 0x0000, bmc830134c.mmc3[0] & (~1));
	chr_swap_BMC830134C(cbase ^ 0x0400, bmc830134c.mmc3[0] |   1);
	chr_swap_BMC830134C(cbase ^ 0x0800, bmc830134c.mmc3[1] & (~1));
	chr_swap_BMC830134C(cbase ^ 0x0C00, bmc830134c.mmc3[1] |   1);
	chr_swap_BMC830134C(cbase ^ 0x1000, bmc830134c.mmc3[2]);
	chr_swap_BMC830134C(cbase ^ 0x1400, bmc830134c.mmc3[3]);
	chr_swap_BMC830134C(cbase ^ 0x1800, bmc830134c.mmc3[4]);
	chr_swap_BMC830134C(cbase ^ 0x1C00, bmc830134c.mmc3[5]);
}
INLINE static void chr_swap_BMC830134C(WORD address, WORD value) {
	WORD base = (bmc830134c.reg & 0x01) << 8;
	WORD mask = 0xFF;

	// Outer Bank Register's CHR A16 and CHR A17 are OR'd with the respective MMC3 bits.
	value = ((bmc830134c.reg & 0x08) << 3) | ((bmc830134c.reg & 0x02) << 6) | value;

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
