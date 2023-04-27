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

void prg_swap_165(WORD address, WORD value);
void chr_fix_165(void);
void chr_swap_165(WORD address, WORD value);

INLINE static BYTE chr_control_165(WORD address);

struct _m165 {
	BYTE reg;
} m165;

void map_init_165(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(165);
	EXTCL_SAVE_MAPPER(165);
	EXTCL_AFTER_RD_CHR(165);
	EXTCL_WR_CHR(165);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(165);
	mapper.internal_struct[0] = (BYTE *)&m165;
	mapper.internal_struct_size[0] = sizeof(m165);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m165, 0x00, sizeof(m165));

	init_MMC3();
	MMC3_prg_swap = prg_swap_165;
	MMC3_chr_fix = chr_fix_165;
	MMC3_chr_swap = chr_swap_165;

	mmc3.reg[0] = 0;
	mmc3.reg[1] = 0;
	mmc3.reg[2] = 0;
	mmc3.reg[4] = 0;

	if (!info.chr.ram.banks_8k_plus) {
		info.chr.ram.banks_8k_plus = 1;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_165(WORD address, BYTE value) {
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
BYTE extcl_save_mapper_165(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m165.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix();
	}

	return (EXIT_OK);
}
void extcl_after_rd_chr_165(WORD address) {
	if (chr_control_165(address)) {
		MMC3_chr_fix();
	}
}
void extcl_update_r2006_165(WORD new_r2006, WORD old_r2006) {
	extcl_update_r2006_MMC3(new_r2006, old_r2006);
	if ((new_r2006 < 0x2000) && chr_control_165(new_r2006)) {
		MMC3_chr_fix();
	}
}
void extcl_wr_chr_165(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (map_chr_ram_slot_in_range(slot)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void prg_swap_165(WORD address, WORD value) {
	prg_swap_MMC3(address, (value & 0x3F));
}
void chr_fix_165(void) {
	WORD bank[2] = { 0, 0 };

	if (!m165.reg) {
		bank[0] = mmc3.reg[0];
		bank[1] = mmc3.reg[2];
	} else if (m165.reg) {
		bank[0] = mmc3.reg[1];
		bank[1] = mmc3.reg[4];
	}

	bank[0] &= ~3;
	chr_swap_165(0x0000, bank[0] | 0x00);
	chr_swap_165(0x0400, bank[0] | 0x01);
	chr_swap_165(0x0800, bank[0] | 0x02);
	chr_swap_165(0x0C00, bank[0] | 0x03);

	bank[1] &= ~3;
	chr_swap_165(0x1000, bank[1] | 0x00);
	chr_swap_165(0x1400, bank[1] | 0x01);
	chr_swap_165(0x1800, bank[1] | 0x02);
	chr_swap_165(0x1C00, bank[1] | 0x03);
}
void chr_swap_165(WORD address, WORD value) {
	const BYTE slot = address >> 10;

	if (!(value & ~3)) {
		control_bank(info.chr.ram.max.banks_1k)
		chr.bank_1k[slot] = &chr.extra.data[value << 10];
	} else {
		chr_swap_MMC3(address, value);
	}
}

INLINE static BYTE chr_control_165(WORD address) {
	address &= 0x1FF0;
	if (address == 0x1FD0) {
		m165.reg = 0;
		return (TRUE);
	}
	if (address == 0x1FE0) {
		m165.reg = 1;
		return (TRUE);
	}
	return (FALSE);
}
