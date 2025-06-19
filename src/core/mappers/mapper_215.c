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

void prg_swap_mmc3_215(WORD address, WORD value);
void chr_swap_mmc3_215(WORD address, WORD value);

struct _m215 {
	BYTE reg[4];
} m215;

void map_init_215(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(215);
	EXTCL_CPU_RD_MEM(215);
	EXTCL_SAVE_MAPPER(215);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	map_internal_struct_init((BYTE *)&m215, sizeof(m215));
	map_internal_struct_init((BYTE *)&mmc3, sizeof(mmc3));

	memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	memset(&m215, 0x00, sizeof(m215));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_215;
	MMC3_chr_swap = chr_swap_mmc3_215;

	m215.reg[1] = 0x03;
	m215.reg[2] = 0x07;
	m215.reg[3] = 0x04;

	if ((info.mapper.submapper < 2) && (prgrom_size() >= S2M)) {
		info.mapper.submapper = 1;
	}

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_215(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		switch (address & 0x0007) {
			case 0:
				m215.reg[0] = value;
				MMC3_prg_fix();
				MMC3_chr_fix();
				return;
			case 1:
				m215.reg[1] = value;
				MMC3_prg_fix();
				MMC3_chr_fix();
				return;
			case 2:
				m215.reg[2] = value & 0x07;
				return;
			case 7:
				m215.reg[3] = value & 0x07;
				break;
			default:
				return;
		}
	}
	if (address >= 0x8000) {
		static const WORD m215_adr[8][8] = {
			{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 },
			{ 0xA001, 0xA000, 0x8000, 0xC000, 0x8001, 0xC001, 0xE000, 0xE001 },
			{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 },
			{ 0xC001, 0x8000, 0x8001, 0xA000, 0xA001, 0xE001, 0xE000, 0xC000 },
			{ 0xA001, 0x8001, 0x8000, 0xC000, 0xA000, 0xC001, 0xE000, 0xE001 },
			{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 },
			{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 },
			{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }
		};
		static const BYTE m215_reg[8][8] = {
			{ 0, 1, 2, 3, 4, 5, 6, 7 },
			{ 0, 2, 6, 1, 7, 3, 4, 5 },
			{ 0, 5, 4, 1, 7, 2, 6, 3 },
			{ 0, 6, 3, 7, 5, 2, 4, 1 },
			{ 0, 2, 5, 3, 6, 1, 7, 4 },
			{ 0, 1, 2, 3, 4, 5, 6, 7 },
			{ 0, 1, 2, 3, 4, 5, 6, 7 },
			{ 0, 1, 2, 3, 4, 5, 6, 7 },
		};

		address = m215_adr[m215.reg[3]][((address >> 12) & 0x06) | (address & 0x01)];

		if (address == 0x8000) {
			value = (value & 0xC0) | m215_reg[m215.reg[3]][value & 0x07];
		}
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_215(BYTE nidx, WORD address, BYTE openbus) {
	static const BYTE arrayLUT[8][8] = {
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0 Super Hang-On
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00 }, // 1 Monkey King
		{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00 }, // 2 Super Hang-On/Monkey King
		{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00 }, // 3 Super Hang-On/Monkey King
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 6
		{ 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0F, 0x00 }  // 7 (default) Blood of Jurassic
	};

	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((openbus & 0xF0) | (arrayLUT[m215.reg[2]][address & 0x07] & 0x0F));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_215(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m215.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_215(WORD address, WORD value) {
	const WORD slot = (address >> 13) & 0x03;
	WORD base = info.mapper.submapper == 1
		? ((m215.reg[1] & 0x03) << 5) | ((m215.reg[1] & 0x08) << 4)
		: ((m215.reg[1] & 0x03) << 5);
	WORD mask = 0x1F >> ((m215.reg[0] & 0x40) >> 6);

	if (dipswitch.used) {
		if (dipswitch.value & 0x01) {
			base &= dipswitch.value;
		} else {
			base |= dipswitch.value;
		}
	}
	base |= (m215.reg[1] & 0x10);
	if (m215.reg[0] & 0x80) {
		value = (m215.reg[0] & 0x0F);
		if (m215.reg[0] & 0x20) {
			base = base & ~3;
			mask = (mask & ~3) | 0x03;
			value = ((value & ~1) << 1) | slot;
		} else {
			base = base & ~1;
			mask = (mask & ~1) | 0x01;
			value = (value << 1) | (slot & 0x01);
		}
	}
	prg_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
void chr_swap_mmc3_215(WORD address, WORD value) {
	WORD base = info.mapper.submapper == 1 ? ((m215.reg[1] & 0x0E) << 7) : ((m215.reg[1] & 0x0C) << 6);
	WORD mask = 0xFF >> ((m215.reg[0] & 0x40) >> 6);

	if (dipswitch.used && (dipswitch.value & 0x01)) {
		base &= (dipswitch.value << 3) | 0x07;
	}
	base |=  ((m215.reg[1] & 0x20) << 2);
	chr_swap_MMC3_base(address, ((base & ~mask) | (value & mask)));
}
