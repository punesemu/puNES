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

void prg_swap_322(WORD address, WORD value);
void chr_swap_322(WORD address, WORD value);

struct _m322 {
	WORD reg;
} m322;

void map_init_322(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(322);
	EXTCL_SAVE_MAPPER(322);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m322;
	mapper.internal_struct_size[0] = sizeof(m322);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m322, 0x00, sizeof(m322));

	init_MMC3();
	MMC3_prg_swap = prg_swap_322;
	MMC3_chr_swap = chr_swap_322;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_322(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			m322.reg = address;
			MMC3_prg_fix(mmc3.bank_to_update);
			MMC3_chr_fix(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_322(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m322.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_322(WORD address, WORD value) {
	BYTE mmc3_mode = m322.reg & 0x0020;
	BYTE mode_128k = !((m322.reg & 0x0080) && mmc3_mode);

	if (mmc3_mode) {
		// modalita' MMC3
		WORD base = (((m322.reg & 0x0040) >> 4) | ((m322.reg & 0x0018) >> 3)) << (5 - mode_128k);
		WORD mask = 0x1F >> mode_128k;

		value = (base & ~mask) | (value & mask);
	} else {
		// NROM mode
		BYTE bank = ((m322.reg & 0x0040) >> 1) | (m322.reg & 0x001f);
		BYTE mode = (m322.reg & 0x03) != 0;

		bank = ((address & 0x4000) ? bank | mode : bank & ~mode);
		value = (bank << 1) | ((address & 0x2000) >> 13);
	}
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_322(WORD address, WORD value) {
	BYTE mmc3_mode = m322.reg & 0x0020;
	BYTE mode_128k = !((m322.reg & 0x0080) && mmc3_mode);
	WORD base = (((m322.reg & 0x0040) >> 4) | ((m322.reg & 0x0018) >> 3)) << (8 - mode_128k);
	WORD mask = 0xFF >> mode_128k;

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
