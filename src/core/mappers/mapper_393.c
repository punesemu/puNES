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
#include "save_slot.h"

void prg_swap_mmc3_393(WORD address, WORD value);
void chr_swap_mmc3_393(WORD address, WORD value);

struct _m393 {
	BYTE reg[2];
} m393;

void map_init_393(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(393);
	EXTCL_SAVE_MAPPER(393);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m393, sizeof(m393));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m393, 0x00, sizeof(m393));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_393;
	MMC3_chr_swap = chr_swap_mmc3_393;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_393(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address))) {
			m393.reg[0] = address & 0xFF;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		m393.reg[1] = value;
		if (m393.reg[0] & 0x30) {
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		if ((address & 0xE001) == 0x8001) {
			switch (mmc3.bank_to_update & 0x07) {
				case 6:
					mmc3.reg[6] = value;

					if ((m393.reg[0] & 0x30) == 0x20) {
						MMC3_prg_fix();
					} else {
						if (mmc3.bank_to_update & 0x40) {
							MMC3_prg_swap(0xC000, value);
						} else {
							MMC3_prg_swap(0x8000, value);
						}
					}
					return;
				default:
					extcl_cpu_wr_mem_MMC3(nidx, address, value);
					return;
			}
		}
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_393(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m393.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_393(WORD address, WORD value) {
	WORD base = (m393.reg[0] & 0x07) << 4;
	WORD mask = 0x0F;

	if (m393.reg[0] & 0x20) {
		if (m393.reg[0] & 0x10) {
			value = ((address & 0x4000 ? 0x07 : m393.reg[1] & 0x07) << 1) | ((address >> 13) & 0x01);
		} else {
			value = (mmc3.reg[6] & 0x0C) | ((address >> 13) & 0x03);
		}
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_393(WORD address, WORD value) {
	if ((m393.reg[0] & 0x08) && vram_size(0)) {
		memmap_vram_1k(0, MMPPU(address), (address >> 10));
	} else {
		WORD base = (m393.reg[0] & 0x07) << 8;
		WORD mask = 0xFF;

		chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
	}
}
