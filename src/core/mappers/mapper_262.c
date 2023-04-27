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

void prg_swap_mmc3_262(WORD address, WORD value);
void chr_swap_mmc3_262(WORD address, WORD value);

struct _m262 {
	BYTE reg;
} m262;
struct _m262tmp {
	BYTE reset;
} m262tmp;

void map_init_262(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(262);
	EXTCL_CPU_RD_MEM(262);
	EXTCL_SAVE_MAPPER(262);
	EXTCL_WR_CHR(262);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m262;
	mapper.internal_struct_size[0] = sizeof(m262);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m262, 0x00, sizeof(m262));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_262;
	MMC3_chr_swap = chr_swap_mmc3_262;

	if (info.reset >= HARD) {
		m262tmp.reset = 0;
	} else if (info.reset == RESET) {
		m262tmp.reset ^= 0xFF;
	}

	info.chr.ram.banks_8k_plus = 1;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_262(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		if (address & 0x0100) {
			m262.reg = value;
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_262(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		if (address & 0x0100) {
			return (m262tmp.reset);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_262(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m262.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_262(WORD address, BYTE value) {
	if ((m262.reg & 0x40) && chr.extra.data) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

void prg_swap_mmc3_262(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_262(WORD address, WORD value) {
	const BYTE slot = address >> 10;
	WORD base = 0;

	if ((m262.reg & 0x40) && chr.extra.data) {
		chr.bank_1k[slot] = &chr.extra.data[slot << 10];
	} else {
		static const BYTE shift[] = { 3, 2, 0, 1 };
		const BYTE index = (slot & 0x06) >> 1;

		base = ((m262.reg >> shift[index]) & 0x01) * 0x100;
		chr_swap_MMC3_base(address, (base | value));
	}
}
