/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

void prg_swap_mmc3_219(WORD address, WORD value);
void chr_swap_mmc3_219(WORD address, WORD value);

struct _m219 {
	BYTE reg[2];
} m219;

void map_init_219(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(219);
	EXTCL_SAVE_MAPPER(219);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m219, sizeof(m219));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	m219.reg[0] = 0xFF;
	m219.reg[1] = 0x00;

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_219;
	MMC3_chr_swap = chr_swap_mmc3_219;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_219(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m219.reg[0] = address & 0x0001
			? (m219.reg[0] & 0xFD) | ((value & 0x20) >> 4)
			: (m219.reg[0] & 0xFE) | ((value & 0x08) >> 3);
		MMC3_prg_fix();
		MMC3_chr_fix();
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(nidx, address, value);
				if (address & 0x0002) {
					m219.reg[1] = value & 0x20;
				}
				return;
			case 0x8001:
				if (!m219.reg[1]) {
					extcl_cpu_wr_mem_MMC3(nidx, address, value);
					return;
				}
				if ((mmc3.bank_to_update >= 0x25) && (mmc3.bank_to_update <= 0x26)) {
					value = ((value >> 5) & 0x01) | ((value >> 3) & 0x02) | ((value >> 1) & 0x04) | ((value << 1) & 0x08);
					mmc3.reg[6 | (mmc3.bank_to_update & 0x01)] = value;
				} else if ((mmc3.bank_to_update >= 0x08) && (mmc3.bank_to_update < 0x20)) {
					int index = (mmc3.bank_to_update - 8) >> 2;

					if (mmc3.bank_to_update & 0x01) {
						mmc3.reg[index] &= 0xFFF0;
						mmc3.reg[index] |= ((value & 0x1E) >> 1);
					} else {
						mmc3.reg[index] &= 0xFF0F;
						mmc3.reg[index] |= ((value & 0x0F) << 4);
					}
				}
				MMC3_prg_fix();
				MMC3_chr_fix();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(nidx, address, value);
				return;
		}
	}
}
BYTE extcl_save_mapper_219(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m219.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_219(WORD address, WORD value) {
	WORD base = m219.reg[0] << 4;
	WORD mask = 0x0F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_219(WORD address, WORD value) {
	WORD base = m219.reg[0] << 7;
	WORD mask = 0x7F;

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
