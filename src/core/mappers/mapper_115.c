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

void prg_fix_115(BYTE value);
void prg_swap_115(WORD address, WORD value);
void chr_swap_115(WORD address, WORD value);

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
		memset(&irqA12, 0x00, sizeof(irqA12));
		memset(&m115, 0x00, sizeof(m115));
	} else {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	init_MMC3();
	MMC3_prg_fix = prg_fix_115;
	MMC3_prg_swap = prg_swap_115;
	MMC3_chr_swap = chr_swap_115;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_115(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			if (address & 0x0001) {
				m115.reg[0] = value;
				MMC3_chr_fix(mmc3.bank_to_update);
			} else {
				m115.reg[1] = value;
				MMC3_prg_fix(mmc3.bank_to_update);
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
					MMC3_prg_fix(mmc3.bank_to_update);
					return;
				default:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
			return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_115(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m115.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_fix_115(BYTE value) {
	if (m115.reg[1] & 0x80) {
		value = (m115.reg[1] & 0x0F);
		if (m115.reg[1] & 0x20) {
			value >>= 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
		map_prg_rom_8k_update();
		return;
	}
	prg_fix_MMC3(value);
}
void prg_swap_115(WORD address, WORD value) {
	control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_115(WORD address, WORD value) {
	WORD base = m115.reg[0] << 8;
	WORD mask = 0xFF;

	value = base | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
