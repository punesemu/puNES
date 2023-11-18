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

void prg_swap_mmc3_012(WORD address, WORD value);
void chr_swap_mmc3_012(WORD address, WORD value);

struct _m012 {
	BYTE reg;
} m012;

void map_init_012(void) {
	if (info.mapper.submapper == 1) {
		map_init_006();
	} else {
		EXTCL_AFTER_MAPPER_INIT(MMC3);
		EXTCL_CPU_WR_MEM(012);
		EXTCL_CPU_RD_MEM(012);
		EXTCL_SAVE_MAPPER(012);
		EXTCL_CPU_EVERY_CYCLE(MMC3);
		EXTCL_PPU_000_TO_34X(MMC3);
		EXTCL_PPU_000_TO_255(MMC3);
		EXTCL_PPU_256_TO_319(MMC3);
		EXTCL_PPU_320_TO_34X(MMC3);
		EXTCL_UPDATE_R2006(MMC3);
		mapper.internal_struct[0] = (BYTE *)&m012;
		mapper.internal_struct_size[0] = sizeof(m012);
		mapper.internal_struct[1] = (BYTE *)&mmc3;
		mapper.internal_struct_size[1] = sizeof(mmc3);

		if (info.reset >= HARD) {
			memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
			memset(&m012, 0x00, sizeof(m012));
		}

		init_MMC3(info.reset);
		MMC3_prg_swap = prg_swap_mmc3_012;
		MMC3_chr_swap = chr_swap_mmc3_012;

		info.mapper.extend_wr = TRUE;

		nes[0].irqA12.present = TRUE;
		irqA12_delay = 1;
	}
}
void extcl_cpu_wr_mem_012(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4000) && (address < 0x4FFF)) {
		if (address & 0x0100) {
			m012.reg = value;
			MMC3_chr_fix();
		}
		return;
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_012(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4000) && (address < 0x4FFF)) {
		return (address & 0x0100 ? dipswitch.value : wram_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_012(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m012.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_012(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_012(WORD address, WORD value) {
	WORD base = (m012.reg << (((address >> 10) >= 4) ? 4 : 8)) & 0x0100;

	chr_swap_MMC3_base(address, (base | (value & 0xFF)));
}
