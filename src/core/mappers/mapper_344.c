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

void prg_swap_344(WORD address, WORD value);
void chr_swap_344(WORD address, WORD value);

static BYTE dipswitch[2] = { 0x00, 0x78 };
static BYTE prg_chip_order[2][4] = {
	{ 0, 1, 2, 3 },
	{ 0, 3, 1, 2 }
};
struct _m344 {
	WORD reg;
} m344;
struct _m344tmp {
	BYTE prg_chip;
	BYTE dipswitch;
} m344tmp;

void map_init_344(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(344);
	EXTCL_CPU_RD_MEM(344);
	EXTCL_SAVE_MAPPER(344);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m344;
	mapper.internal_struct_size[0] = sizeof(m344);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m344, 0x00, sizeof(m344));

	init_MMC3();
	MMC3_prg_swap = prg_swap_344;
	MMC3_chr_swap = chr_swap_344;

	m344tmp.prg_chip = info.crc32.prg == 0xAB2ACA46 ? 1 : 0;
	if (info.reset == RESET) {
		m344tmp.dipswitch = !m344tmp.dipswitch;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m344tmp.dipswitch = 0;
	}

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_344(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			m344.reg = address;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		if ((address & 0xE001) == 0x8001) {
			switch (mmc3.bank_to_update & 0x07) {
				case 6:
					mmc3.reg[6] = value;
					if (m344.reg & 0x04) {
						MMC3_prg_fix();
					} else {
						if (mmc3.bank_to_update & 0x40) {
							MMC3_prg_swap(0xC000, value);
						} else {
							MMC3_prg_swap(0x8000, value);
						}
					}
					return;
				default:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_344(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (m344.reg & 0x08)) {
		return (dipswitch[m344tmp.dipswitch]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_344(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m344.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_344(WORD address, WORD value) {
	WORD base = (prg_chip_order[m344tmp.prg_chip][m344.reg & 0x03]) << 4;
	WORD mask = 0x0F;

	// NROM mode
	if (m344.reg & 0x04) {
		base = base | (mmc3.reg[6] & 0x0C);
		value = (address >> 13) & 0x03;
		mask = 0x03;
	}
	prg_swap_MMC3(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_344(WORD address, WORD value) {
	WORD base = (m344.reg & 0x03) << 7;
	WORD mask = m344.reg & 0x02 ? 0x7F : 0xFF;

	chr_swap_MMC3(address, (base | (value & mask)));
}
