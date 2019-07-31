/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#define tqrom_8000_swap_chr_ram_1k(slot0, slot1)\
{\
	uint32_t save[1][2];\
	save[0][0] = txrom.chr[slot0][0];\
	save[0][1] = txrom.chr[slot0][1];\
	txrom.chr[slot0][0] = txrom.chr[slot1][0];\
	txrom.chr[slot0][1] = txrom.chr[slot1][1];\
	txrom.chr[slot1][0] = save[0][0];\
	txrom.chr[slot1][1] = save[0][1];\
}
#define tqrom_8001_swap_chr_2k(slot0, slot1)\
{\
	const BYTE a = slot0;\
	const BYTE b = slot1;\
	if (value & 0x40) {\
		txrom.chr[a][0] = txrom.chr[b][0] = TRUE;\
		value >>= 1;\
		control_bank(3)\
		txrom.chr[a][1] = value << 11;\
		txrom.chr[b][1] = txrom.chr[a][1] | 0x400;\
		chr.bank_1k[a] = &chr.extra.data[txrom.chr[a][1]];\
		chr.bank_1k[b] = &chr.extra.data[txrom.chr[b][1]];\
		return;\
	} else {\
		txrom.chr[a][0] = txrom.chr[b][0] = FALSE;\
		txrom.chr[a][1] = txrom.chr[b][1] = FALSE;\
	}\
}
#define tqrom_8001_swap_chr_1k(slot0)\
{\
	const BYTE a = slot0;\
	if (value & 0x40) {\
		txrom.chr[a][0] = TRUE;\
		control_bank(7)\
		txrom.chr[a][1] = value << 10;\
		chr.bank_1k[a] = &chr.extra.data[txrom.chr[a][1]];\
		return;\
	} else {\
		txrom.chr[a][0] = txrom.chr[a][1] = FALSE;\
	}\
}

BYTE type;

void map_init_TxROM(BYTE model) {
	switch (model) {
		case TLSROM:
		case TKSROM:
			EXTCL_CPU_WR_MEM(TKSROM);

			irqA12_delay = 1;

			if (model == TKSROM) {
				info.prg.ram.banks_8k_plus = 1;
				info.prg.ram.bat.banks = 1;
			}
			break;
		case TQROM:
			EXTCL_CPU_WR_MEM(TQROM);
			EXTCL_WR_CHR(TQROM);

			/* utilizza 0x2000 di CHR RAM extra */
			map_chr_ram_extra_init(0x2000);

			mapper.write_vram = FALSE;

			irqA12_delay = 1;

			break;
	}

	EXTCL_SAVE_MAPPER(TxROM);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &txrom;
	mapper.internal_struct_size[0] = sizeof(txrom);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&txrom.chr, 0x00, sizeof(txrom.chr));
		memset(&mmc3, 0x00, sizeof(mmc3));
		map_chr_ram_extra_reset();
	}

	memset(&irqA12, 0x00, sizeof(irqA12));
	txrom.delay = 0;

	irqA12.present = TRUE;

	type = model;
}

void extcl_cpu_wr_mem_TKSROM(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8001: {
			switch (mmc3.bank_to_update) {
				case 0:
				case 1:
					if (!mmc3.chr_rom_cfg) {
						const BYTE slot = mmc3.bank_to_update << 1;

						ntbl.bank_1k[slot] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
						ntbl.bank_1k[slot | 0x01] = ntbl.bank_1k[slot];
					}
					break;
				case 2:
				case 3:
				case 4:
				case 5:
					if (mmc3.chr_rom_cfg) {
						ntbl.bank_1k[mmc3.bank_to_update - 2] = &ntbl.data[((value >> 7) ^ 0x01)
							<< 10];
					}
					break;
			}
			break;
		}
		case 0xA000:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}

void extcl_cpu_wr_mem_TQROM(WORD address, BYTE value) {
	const WORD adr = address & 0xE001;

	if (adr == 0x8000) {
		if (mmc3.chr_rom_cfg != ((value & 0x80) >> 5)) {
			tqrom_8000_swap_chr_ram_1k(0, 4)
			tqrom_8000_swap_chr_ram_1k(1, 5)
			tqrom_8000_swap_chr_ram_1k(2, 6)
			tqrom_8000_swap_chr_ram_1k(3, 7)
		}
	} else if (adr == 0x8001) {
		switch (mmc3.bank_to_update) {
			case 0:
				tqrom_8001_swap_chr_2k(mmc3.chr_rom_cfg, mmc3.chr_rom_cfg | 0x01)
				break;
			case 1:
				tqrom_8001_swap_chr_2k(mmc3.chr_rom_cfg | 0x02, mmc3.chr_rom_cfg | 0x03)
				break;
			case 2:
				tqrom_8001_swap_chr_1k(mmc3.chr_rom_cfg ^ 0x04)
				break;
			case 3:
				tqrom_8001_swap_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x01)
				break;
			case 4:
				tqrom_8001_swap_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x02)
				break;
			case 5:
				tqrom_8001_swap_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x03)
				break;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
void extcl_wr_chr_TQROM(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (txrom.chr[slot]) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

BYTE extcl_save_mapper_TxROM(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, txrom.delay);
	if (type == TQROM) {
		save_slot_ele(mode, slot, txrom.chr);
		if (mode == SAVE_SLOT_READ) {
			BYTE i;

			for (i = 0; i < LENGTH(txrom.chr); i++) {
				if (txrom.chr[i][0]) {
					chr.bank_1k[i] = &chr.extra.data[txrom.chr[i][1]];
				}
			}
		}
		save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
	}
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
