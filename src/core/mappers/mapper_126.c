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

void prg_swap_mmc3_126(WORD address, WORD value);
void chr_swap_mmc3_126(WORD address, WORD value);

static const BYTE dipswitch_126[4] = { 0, 1, 2, 3 };
static const SBYTE dipswitch_index_126[][4] = {
	{ 0,  1,  2,  3 }, // 0
	{ 0,  1, -1, -1 }, // 1
	{ 0,  2,  3,  1 }, // 2
};

struct _m126 {
	BYTE reg[4];
} m126;
struct _m126tmp {
	BYTE model;
	BYTE select;
	BYTE index;
	BYTE dipswitch;
} m126tmp;

void map_init_126(BYTE model) {
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

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m126, 0x00, sizeof(m126));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_126;
	MMC3_chr_swap = chr_swap_mmc3_126;

	if (info.reset == RESET) {
		do {
			m126tmp.index = (m126tmp.index + 1) & 0x03;
		} while (dipswitch_index_126[m126tmp.select][m126tmp.index] < 0);
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (info.crc32.prg == 0xB1082DE6) { // 1998 4000000-in-1 (BS-400 PCB).nes
			m126tmp.select = 2;
			m126tmp.index = 0;
		} else if (
			(info.crc32.prg == 0xA4AEEA4A) || // 3000000-in-1 (BS-300 PCB).nes
			(info.crc32.prg == 0xB5EC8A0A)) { // 700000-in-1 (BS-400 PCB).nes
			m126tmp.select = 1;
			m126tmp.index = 0;
		} else if (
			(info.crc32.prg == 0xC6FDE109) || // Double Dragon 530-in-1.nes
			(info.crc32.prg == 0x9E54027F)) { // (GD-106) 18-in-1.nes
			m126tmp.select = 1;
			m126tmp.index = 1;
		} else {
			m126tmp.select = 0;
			m126tmp.index = 0;
		}
	}
	m126tmp.dipswitch = dipswitch_126[dipswitch_index_126[m126tmp.select][m126tmp.index]];
	m126tmp.model = model;

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = (m126tmp.model == MAP534) ? 2: 1;
}
void extcl_cpu_wr_mem_126(WORD address, BYTE value) {
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
				if (m126tmp.model == MAP534) {
					value ^= 0xFF;
				}
				break;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_126(WORD address, BYTE openbus) {
	if ((address >= 0x8000) && (m126.reg[1] & 0x01)) {
		return ((openbus & 0xFC) | m126tmp.dipswitch);
	}
	return (openbus);
}
BYTE extcl_save_mapper_126(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m126.reg);
	save_slot_ele(mode, slot, m126tmp.index);
	save_slot_ele(mode, slot, m126tmp.dipswitch);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_126(WORD address, WORD value) {
	WORD base = ((m126.reg[0] & 0x30) << 3) | ((m126.reg[0] & 0x07) << 4);
	WORD mask = ((~m126.reg[0] & 0x40) >> 2) | 0x0F;
	BYTE bank = (address >> 13) & 0x03;

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
	WORD base = m126tmp.model == MAP126 ?
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
