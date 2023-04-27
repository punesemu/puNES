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

void prg_swap_353(WORD address, WORD value);
void chr_swap_353(WORD address, WORD value);
void mirroring_fix_353(void);

INLINE static void mirroring_swap_353(BYTE slot, BYTE value);

struct _m353 {
	BYTE reg;
	BYTE chr_writable;
} m353;

void map_init_353(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(353);
	EXTCL_SAVE_MAPPER(353);
	EXTCL_WR_CHR(353);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m353;
	mapper.internal_struct_size[0] = sizeof(m353);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m353, 0x00, sizeof(m353));

	init_MMC3();
	MMC3_prg_swap = prg_swap_353;
	MMC3_chr_swap = chr_swap_353;
	MMC3_mirroring_fix = mirroring_fix_353;

	if (info.format != NES_2_0) {
		info.chr.ram.banks_8k_plus = 1;
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_353(WORD address, BYTE value) {
	if ((address & 0x0FFF) == 0x0080) {
		m353.reg = (address >> 13) & 0x03;
		MMC3_prg_fix();
		MMC3_chr_fix();
		MMC3_mirroring_fix();
		return;
	}
	switch (address & 0xE001) {
		case 0x8000:
			mmc3.bank_to_update = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
			MMC3_mirroring_fix();
			return;
		case 0x8001:
			mmc3.reg[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					MMC3_prg_fix();
					MMC3_chr_fix();
					MMC3_mirroring_fix();
					return;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					MMC3_chr_fix();
					MMC3_mirroring_fix();
					return;
				default:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_353(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m353.reg);
	save_slot_ele(mode, slot, m353.chr_writable);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		MMC3_chr_fix();
		MMC3_mirroring_fix();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_353(WORD address, BYTE value) {
	if (m353.chr_writable) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

void prg_swap_353(WORD address, WORD value) {
	WORD base = m353.reg << 5;
	WORD mask = 0x1F;

	if (m353.reg == 2) {
		base |= (mmc3.reg[0] & 0x80 ? 0x10 : 0x00);
		mask >>= 1;
	} else if ((m353.reg == 3) && !(mmc3.reg[0] & 0x80) && (address >= 0xC000)) {
		base = 0x70;
		mask = 0x0F;
		value = mmc3.reg[address >> 13];
	}
	prg_swap_MMC3(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_353(WORD address, WORD value) {
	WORD base = m353.reg << 7;
	WORD mask = 0x7F;

	if ((m353.reg == 2) && (mmc3.reg[0] & 0x80) && chr.extra.data) {
		m353.chr_writable = TRUE;
		value = address >> 10;
		control_bank(info.chr.ram.max.banks_1k)
		chr.bank_1k[address >> 10] = &chr.extra.data[value << 10];
	} else {
		m353.chr_writable = FALSE;
		value = (base & ~mask) | (value & mask);
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[address >> 10] = chr_pnt(value << 10);
	}
}
void mirroring_fix_353(void) {
	if (!m353.reg) {
		 if (!(mmc3.reg[0] & 0x80)) {
			mirroring_swap_353(0, mmc3.reg[0]);
			mirroring_swap_353(1, mmc3.reg[0]);
			mirroring_swap_353(2, mmc3.reg[1]);
			mirroring_swap_353(3, mmc3.reg[1]);
		 } else {
			mirroring_swap_353(0, mmc3.reg[2]);
			mirroring_swap_353(1, mmc3.reg[3]);
			mirroring_swap_353(2, mmc3.reg[4]);
			mirroring_swap_353(3, mmc3.reg[5]);
		 }
	} else {
		mirroring_fix_MMC3();
	}
}

INLINE static void mirroring_swap_353(BYTE slot, BYTE value) {
	ntbl.bank_1k[slot] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
}
