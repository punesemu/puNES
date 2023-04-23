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

void prg_swap_516(WORD address, WORD value);
void chr_swap_516(WORD address, WORD value);

struct _m516 {
	BYTE reg;
} m516;

void map_init_516(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(516);
	EXTCL_SAVE_MAPPER(516);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m516;
	mapper.internal_struct_size[0] = sizeof(m516);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m516, 0x00, sizeof(m516));

	init_MMC3();
	MMC3_prg_swap = prg_swap_516;
	MMC3_chr_swap = chr_swap_516;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_516(WORD address, BYTE value) {
	if (address & 0x0010) {
		m516.reg = address & 0x0F;
		MMC3_prg_fix(mmc3.bank_to_update);
		MMC3_chr_fix(mmc3.bank_to_update);
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_516(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m516.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_516(WORD address, WORD value) {
	WORD base = (m516.reg & 0x03) << 4;
	WORD mask = 0x0F;

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_516(WORD address, WORD value) {
	WORD base = (m516.reg & 0x0C) << 5;
	WORD mask = 0x7F;

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
