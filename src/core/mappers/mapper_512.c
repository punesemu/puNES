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

void chr_swap_512(WORD address, WORD value);

struct _m512 {
	BYTE reg;
	BYTE vram[0x1000];
} m512;

void map_init_512(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(512);
	EXTCL_SAVE_MAPPER(512);
	EXTCL_WR_CHR(512);
	EXTCL_WR_NMT(512);
	EXTCL_RD_NMT(512);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m512;
	mapper.internal_struct_size[0] = sizeof(m512);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m512, 0x00, sizeof(m512));

	init_MMC3();
	MMC3_chr_swap = chr_swap_512;

	if (info.format != NES_2_0) {
		info.prg.ram.banks_8k_plus = 1;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_512(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if ((address & 0x1100) == 0x0100) {
			m512.reg = value & 0x03;
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_512(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m512.reg);
	save_slot_ele(mode, slot, m512.vram);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_512(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if ((m512.reg > 1) && map_chr_ram_slot_in_range(slot)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}
void extcl_wr_nmt_512(WORD address, BYTE value) {
	address &= 0x0FFF;
	if (m512.reg == 1) {
		m512.vram[address] = value;
		return;
	}
	ntbl.bank_1k[address >> 10][address & 0x3FF] = value;
}
BYTE extcl_rd_nmt_512(WORD address) {
	address &= 0x0FFF;
	if (m512.reg == 1) {
		return (m512.vram[address]);
	}
	return (ntbl.bank_1k[address >> 10][address & 0x3FF]);
}

void chr_swap_512(WORD address, WORD value) {
	if ((m512.reg > 1) && chr.extra.data) {
		control_bank_with_AND(0x03, info.chr.ram.max.banks_1k)
		chr.bank_1k[address >> 10] = &chr.extra.data[value << 10];
	} else {
		control_bank_with_AND(0x7F, info.chr.rom.max.banks_1k)
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	}
}
