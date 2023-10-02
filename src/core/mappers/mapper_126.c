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

void prg_swap_mmc3_126(WORD address, WORD value);
void chr_swap_mmc3_126(WORD address, WORD value);

struct _m126 {
	BYTE reg[4];
} m126;

void map_init_126(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(126);
	EXTCL_CPU_RD_MEM(126);
	EXTCL_SAVE_MAPPER(126);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m126;
	mapper.internal_struct_size[0] = sizeof(m126);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m126, 0x00, sizeof(m126));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_126;
	MMC3_chr_swap = chr_swap_mmc3_126;

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = (info.mapper.id == 534) ? 2: 1;
}
void extcl_cpu_wr_mem_126(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		const BYTE reg = address & 0x03;

		if (!(m126.reg[3] & 0x80)) {
			m126.reg[reg] = value;
			MMC3_prg_fix();
		} else if (reg == 2) {
			const BYTE mask = 0x03 >> ((m126.reg[2] & 0x10) != 0);

			m126.reg[2] = (m126.reg[2] & ~mask) | (value & mask);
		}
		MMC3_chr_fix();
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0xC000:
			case 0xC001:
				if (info.mapper.id == 534) {
					value ^= 0xFF;
				}
				break;
		}
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_126(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m126.reg[1] & 0x01
			? (prgrom_rd(nidx, address) & 0xFC) | dipswitch.value
			: prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_126(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m126.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_126(WORD address, WORD value) {
	WORD mask = ((~m126.reg[0] & 0x40) >> 2) | 0x0F;
	WORD base = (((m126.reg[0] & 0x30) << 3) | ((m126.reg[0] & 0x07) << 4)) & ~mask;
	BYTE bank = (address >> 13) & 0x03;

	if (info.mapper.submapper == 1) {
		base = base | ((base & 0x100) >> 1);
	}

	switch (m126.reg[3] & 0x03) {
		case 1:
		case 2:
			base = base | (mmc3.reg[6] & mask);
			mask = 0x01;
			value = bank & 0x01;
			break;
		case 3:
			base = base | (mmc3.reg[6] & mask);
			mask = 0x03;
			value = bank;
			break;
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_126(WORD address, WORD value) {
	WORD base = info.mapper.id == 126 ?
		((m126.reg[0] & 0x08) << 4) | ((m126.reg[0] & 0x20) << 3) | ((m126.reg[0] & 0x10) << 5) :
		((m126.reg[0] & 0x38) << 4);
	WORD mask = (~m126.reg[0] & 0x80) | 0x7F;

	if (m126.reg[3] & 0x10) {
		base = ((m126.reg[2] & (mask >> 3)) | (base >> 3)) << 3;
		mask = 0x07;
		value = address >> 10;
	}
	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
