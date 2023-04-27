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

void prg_swap_mmc3_121(WORD address, WORD value);
void chr_swap_mmc3_121(WORD address, WORD value);

struct _m121 {
	BYTE reg[7];
} m121;

void map_init_121(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(121);
	EXTCL_CPU_RD_MEM(121);
	EXTCL_SAVE_MAPPER(121);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m121;
	mapper.internal_struct_size[0] = sizeof(m121);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m121, 0x00, sizeof(m121));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_121;
	MMC3_chr_swap = chr_swap_mmc3_121;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_121(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		static const BYTE vlu121[] = { 0x83, 0x83, 0x42, 0x00, 0x00, 0x02, 0x02, 0x03 };

		m121.reg[4] = vlu121[(((address >> 6) & 0x04) | (value & 0x03))];
		if (address & 0x0100) {
			m121.reg[3] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		if ((address & 0xE001) == 0x8001) {
			mmc3.reg[mmc3.bank_to_update & 0x07] = value;
			if ((address & 0x03) == 0x03) {
				m121.reg[5] = value & 0x3F;
				m121.reg[2] = (m121.reg[5] & 0x20) && m121.reg[6] ? m121.reg[6] : m121.reg[2];
			} else {
				m121.reg[6] =
					((value & 0x01) << 5) | ((value & 0x02) << 3) |
					((value & 0x04) << 1) | ((value & 0x08) >> 1) |
					((value & 0x10) >> 3) | ((value & 0x20) >> 5);
				if ((m121.reg[5] == 0x26) || (m121.reg[5] == 0x28) || (m121.reg[5] == 0x2A)) {
					m121.reg[0x15 - (m121.reg[5] >> 1)] = m121.reg[6];
				}
			}
			MMC3_prg_fix();
			MMC3_chr_fix();
			return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_121(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (m121.reg[4]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_121(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m121.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_121(WORD address, WORD value) {
	const BYTE slot = (address >> 13) & 0x03;
	WORD base = (m121.reg[3] & 0x80) >> 2;
	WORD mask = 0x1F;

	if ((m121.reg[5] & 0x20) && slot) {
		value = m121.reg[slot - 1];
		mask = 0xFF;
	}

	value = base | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, slot, value);
	map_prg_rom_8k_update();
}
void chr_swap_mmc3_121(WORD address, WORD value) {
	const BYTE slot = address >> 10;
	WORD base = 0;

	if (prg_size() > (1024 * 256)) {
		base = (m121.reg[3] & 0x80) << 1;
	} else if ((slot & 0x04) == ((mmc3.bank_to_update & 0x80) >> 5)) {
		base = 0x100;
	}
	chr_swap_MMC3_base(address, (base | value));
}
