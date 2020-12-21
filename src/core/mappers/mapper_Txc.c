/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

struct _t22211x {
	BYTE reg[4];
} t22211x;
struct _txctmp {
	BYTE type;
} txctmp;

void map_init_Txc(BYTE model) {
	switch (model) {
		case TXCTW:
			EXTCL_CPU_WR_MEM(Txc_tw);
			EXTCL_SAVE_MAPPER(MMC3);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *) &mmc3;
			mapper.internal_struct_size[0] = sizeof(mmc3);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				/* sembra proprio che parta in questa modalita' */
				mmc3.prg_rom_cfg = 0x02;
				map_prg_rom_8k(4, 0, 0);
			}

			memset(&irqA12, 0x00, sizeof(irqA12));

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case T22211A:
		case T22211B:
		case T22211C:
			EXTCL_CPU_WR_MEM(Txc_t22211x);
			EXTCL_CPU_RD_MEM(Txc_t22211x);
			EXTCL_SAVE_MAPPER(Txc_t22211x);
			mapper.internal_struct[0] = (BYTE *) &t22211x;
			mapper.internal_struct_size[0] = sizeof(t22211x);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&t22211x, 0x00, sizeof(t22211x));
				if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
					map_prg_rom_8k(4, 0, 0);
				}
			}
			break;
	}

	txctmp.type = model;
}

void extcl_cpu_wr_mem_Txc_tw(WORD address, BYTE value) {
	if (address < 0x4120) {
		return;
	}

	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	value = (value >> 4) | value;
	control_bank(info.prg.rom[0].max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Txc_t22211x(WORD address, BYTE value) {
	BYTE save = value;

	if ((address >= 0x4100) && (address <= 0x4103)) {
		t22211x.reg[address & 0x0003] = value;
		return;
	}

	if (address < 0x8000) {
		return;
	}

	if (info.prg.rom[0].max.banks_32k != 0xFFFF) {
		value = t22211x.reg[2] >> 2;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
	}

	{
		DBWORD bank;

		if (txctmp.type == T22211B) {
			value = (((save ^ t22211x.reg[2]) >> 3) & 0x02)
				| (((save ^ t22211x.reg[2]) >> 5) & 0x01);
		} else {
			value = t22211x.reg[2];
		}

		control_bank(info.chr.rom[0].max.banks_8k)
		bank = value << 13;

		chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
		chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
		chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
		chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
		chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
		chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
		chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
		chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);

	}
}
BYTE extcl_cpu_rd_mem_Txc_t22211x(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address != 0x4100) {
		return (openbus);
	}

	if (txctmp.type == T22211C) {
		return ((t22211x.reg[1] ^ t22211x.reg[2]) | 0x41);
	} else {
		return ((t22211x.reg[1] ^ t22211x.reg[2]) | 0x40);
	}
}
BYTE extcl_save_mapper_Txc_t22211x(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, t22211x.reg);

	return (EXIT_OK);
}
