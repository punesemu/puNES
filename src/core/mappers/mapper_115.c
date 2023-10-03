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
#include "irqA12.h"
#include "save_slot.h"

void prg_fix_mmc3_115(void);
void prg_swap_mmc3_115(WORD address, WORD value);
void chr_swap_mmc3_115(WORD address, WORD value);

struct _m115 {
	BYTE reg[2];
} m115;

void map_init_115(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(115);
	EXTCL_SAVE_MAPPER(115);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m115;
	mapper.internal_struct_size[0] = sizeof(m115);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
		memset(&m115, 0x00, sizeof(m115));
	}

	init_MMC3(info.reset);
	MMC3_prg_fix = prg_fix_mmc3_115;
	MMC3_prg_swap = prg_swap_mmc3_115;
	MMC3_chr_swap = chr_swap_mmc3_115;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_115(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			if (address & 0x0001) {
				m115.reg[0] = value;
				MMC3_chr_fix();
			} else {
				m115.reg[1] = value;
				MMC3_prg_fix();
			}
		}
		return;
	}
	if (address >= 0x8000) {
		if ((address & 0xE001) == 0x8001) {
			switch (mmc3.bank_to_update & 0x07) {
				case 6:
				case 7:
					mmc3.reg[mmc3.bank_to_update & 0x07] = value;
					MMC3_prg_fix();
					return;
				default:
					extcl_cpu_wr_mem_MMC3(nidx, address, value);
					return;
			}
			return;
		}
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_115(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m115.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_fix_mmc3_115(void) {
	BYTE value = 0;

	if (m115.reg[1] & 0x80) {
		value = (m115.reg[1] & 0x0F);
		if (m115.reg[1] & 0x20) {
			memmap_auto_32k(0, MMCPU(0x8000), (value >> 1));
		} else {
			memmap_auto_16k(0, MMCPU(0x8000), value);
			memmap_auto_16k(0, MMCPU(0xC000), value);
		}
		return;
	}
	prg_fix_MMC3_base();
}
void prg_swap_mmc3_115(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_115(WORD address, WORD value) {
	chr_swap_MMC3_base(address, ((m115.reg[0] << 8) | (value & 0xFF)));
}

