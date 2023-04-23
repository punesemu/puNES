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

void prg_swap_410(WORD address, WORD value);
void chr_swap_410(WORD address, WORD value);

struct _m410 {
	BYTE index;
	BYTE reg[4];
} m410;

void map_init_410(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(410);
	EXTCL_SAVE_MAPPER(410);
	EXTCL_WR_CHR(410);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m410;
	mapper.internal_struct_size[0] = sizeof(m410);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m410, 0x00, sizeof(m410));

	init_MMC3();
	MMC3_prg_swap = prg_swap_410;
	MMC3_chr_swap = chr_swap_410;

	if (info.format != NES_2_0) {
		info.chr.ram.banks_8k_plus = 1;
	}

	m410.reg[2] = 0x0F;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_410(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active && !(m410.reg[3] & 0x40)) {
			m410.reg[m410.index] = value;
			m410.index = (m410.index + 1) & 0x03;
			MMC3_prg_fix(mmc3.bank_to_update);
			MMC3_chr_fix(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_410(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m410.index);
	save_slot_ele(mode, slot, m410.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix(mmc3.bank_to_update);
	}

	return (EXIT_OK);
}
void extcl_wr_chr_410(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if ((m410.reg[2] & 0x40) && map_chr_ram_slot_in_range(slot)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void prg_swap_410(WORD address, WORD value) {
	WORD base = m410.reg[1] | ((m410.reg[2] & 0xC0) << 2);
	WORD mask = ~m410.reg[3] & 0x3F;

	value = base | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_410(WORD address, WORD value) {
	if ((m410.reg[2] & 0x40) && chr.extra.data) {
		value = address >> 10;
		chr.bank_1k[address >> 10] = &chr.extra.data[value << 10];
	} else {
		WORD base = m410.reg[0] | (m410.reg[2] & 0xF0) << 4;
		WORD mask = 0xFF >> (~m410.reg[2] & 0x0F);

		value = base | (value & mask);
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	}
}
