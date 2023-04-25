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

void prg_swap_333(WORD address, WORD value);
void chr_swap_333(WORD address, WORD value);

struct _m333 {
	BYTE reg;
} m333;

void map_init_333(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(333);
	EXTCL_SAVE_MAPPER(333);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m333;
	mapper.internal_struct_size[0] = sizeof(m333);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m333, 0x00, sizeof(m333));

	init_MMC3();
	MMC3_prg_swap = prg_swap_333;
	MMC3_chr_swap = chr_swap_333;

	if (mapper.write_vram && !info.chr.rom.banks_8k) {
		info.chr.rom.banks_8k = 32;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_333(WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x9000:
		case 0x9001:
		case 0xB000:
		case 0xB001:
		case 0xD000:
		case 0xD001:
		case 0xF000:
		case 0xF001:
			m333.reg = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_333(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m333.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_333(WORD address, WORD value) {
	if (m333.reg & 0x10) {
		value = ((m333.reg & 0x0C) << 2) | (value & 0x0F);
	} else {
		value = ((m333.reg & 0x0F) << 2) | ((address >> 13) & 0x03);
	}
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_333(WORD address, WORD value) {
	value = ((m333.reg & 0x0C) << 5) | (value & 0x7F);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
