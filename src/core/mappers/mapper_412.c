/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

void prg_swap_mmc3_412(WORD address, WORD value);
void chr_swap_mmc3_412(WORD address, WORD value);

struct _m412 {
	BYTE reg[4];
} m412;

void map_init_412(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(412);
	EXTCL_CPU_RD_MEM(412);
	EXTCL_SAVE_MAPPER(412);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m412, sizeof(m412));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m412, 0x00, sizeof(m412));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_412;
	MMC3_chr_swap = chr_swap_mmc3_412;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_412(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x7FFF)) {
		if (memmap_adr_is_writable(nidx, MMCPU(address)) && !(m412.reg[1] & 0x01)) {
			m412.reg[address & 0x0003] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		wram_wr(nidx, address, value);
	} else if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_412(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (dipswitch.used && (address >= 0x5000) && (address <= 0x6FFF)) {
		return (dipswitch.value & 0x0F);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_412(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m412.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_412(WORD address, WORD value) {
	WORD base = 0;
	WORD mask = 0;

	if (m412.reg[2] & 0x02) {
		BYTE n256 = (m412.reg[2] & 0x04) >> 2;

		base = m412.reg[2] >> 3;
		base = (!(address & 0x4000) ? base & ~n256 : base | n256) << 1;
		mask = 0x01;
		value = (address >> 13) & 0x01;
	} else {
		mask = (((~m412.reg[1] & 0x02) << 4) | (~m412.reg[1] & 0x10) | 0x0F);
		base = (((m412.reg[1] & 0x40) >> 2) | ((m412.reg[1] & 0x04) <<  3)) & ~mask;
	}
	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_412(WORD address, WORD value) {
	WORD base = ((m412.reg[1] & 0x08) << 5) | (m412.reg[1] & 0x80);
	WORD mask = 0xFF >> ((m412.reg[1] & 0x20) >> 5);

	if (m412.reg[2] & 0x02) {
		base = (m412.reg[0] >> 2) << 3;
		mask = 0x07;
		value = address >> 10;
	}
	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
