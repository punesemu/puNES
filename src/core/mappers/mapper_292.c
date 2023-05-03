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

void prg_swap_mmc3_292(WORD address, WORD value);
void chr_fix_mmc3_292(void);

struct _m292 {
	BYTE reg[3];
} m292;

void map_init_292(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(292);
	EXTCL_CPU_RD_MEM(292);
	EXTCL_SAVE_MAPPER(292);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m292;
	mapper.internal_struct_size[0] = sizeof(m292);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m292, 0x00, sizeof(m292));
	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_292;
	MMC3_chr_fix = chr_fix_mmc3_292;

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_292(WORD address, BYTE value) {
	if ((address & 0xE001) == 0x6000) {
		if (cpu.prg_ram_wr_active) {
			m292.reg[0] = value;
			MMC3_prg_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		if ((address & 0xE001) == 0x8001) {
			switch (mmc3.bank_to_update & 0x07) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					mmc3.reg[mmc3.bank_to_update & 0x07] = value;
					MMC3_chr_fix();
					return;
				default:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_292(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address & 0xE001) == 0x6000) {
		if (cpu.prg_ram_rd_active) {
			if ((m292.reg[0] & 0xE0) == 0xC0) {
				m292.reg[1] = mmcpu.ram[0x06A];
			} else {
				m292.reg[2] = mmcpu.ram[0x0FF];
			}
			MMC3_chr_fix();
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_292(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m292.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_292(WORD address, WORD value) {
	const BYTE slot = (address >> 13) & 0x03;

	if (!slot) {
		value = m292.reg[0] & 0x1F;
	}
	prg_swap_MMC3_base(address, value);
}
void chr_fix_mmc3_292(void) {
	map_chr_rom_2k(0x0000, ((mmc3.reg[0] >> 1) ^ m292.reg[1]));
	map_chr_rom_2k(0x0800, ((mmc3.reg[1] >> 1) | ((m292.reg[2] & 0x40) << 1)));
	map_chr_rom_4k(0x1000, m292.reg[2] & 0x3F);
}
