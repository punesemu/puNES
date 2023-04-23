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
#include "cpu.h"
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_348(WORD address, WORD value);
void chr_swap_348(WORD address, WORD value);

struct _m348 {
	BYTE reg;
} m348;

void map_init_348(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(348);
	EXTCL_SAVE_MAPPER(348);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m348;
	mapper.internal_struct_size[0] = sizeof(m348);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m348, 0x00, sizeof(m348));

	init_MMC3();
	MMC3_prg_swap = prg_swap_348;
	MMC3_chr_swap = chr_swap_348;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_348(WORD address, BYTE value) {
	if ((address >= 0x6800) && (address <= 0x68FF)) {
		if (cpu.prg_ram_wr_active) {
			m348.reg = value;
			MMC3_prg_fix(mmc3.bank_to_update);
			MMC3_chr_fix(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_348(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m348.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_348(WORD address, WORD value) {
	WORD base = (m348.reg & 0x0C) << 2;
	WORD mask = 0x0F;

	// GNROM mode
	if ((m348.reg & 0x0C) == 0x0C) {
		BYTE bank = (address >> 13) & 0x03;
		BYTE reg = mmc3.reg[0x06 | (bank & 0x01)];

		value = bank < 2 ? reg & 0xFD : reg | 0x02;
	}

	value = base | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_348(WORD address, WORD value) {
	WORD base = (m348.reg & 0x0C) << 5;
	WORD mask = 0x7F;

	value = base | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
