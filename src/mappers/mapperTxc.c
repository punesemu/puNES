/*
 * mapperTxROM.c
 *
 *  Created on: 30/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom8kMax, prgRom8kBeforeLast, chrRom8kMax, chrRom1kMax;
BYTE type;

void map_init_Txc(BYTE model) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom8kMax = info.prg_rom_8k_count - 1;
	chrRom1kMax = info.chr_rom_1k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;
	prgRom8kBeforeLast = info.prg_rom_8k_count - 2;

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

			info.mapper_extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				/* sembra proprio che parta in questa modalita' */
				mmc3.prgRomCfg = 0x02;
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

			info.mapper_extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&t22211x, 0x00, sizeof(t22211x));
				if (prgRom32kMax != 0xFFFF) {
					map_prg_rom_8k(4, 0, 0);
				}
			}
			break;
	}

	type = model;
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
	control_bank(prgRom32kMax)
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

	if (prgRom32kMax != 0xFFFF) {
		value = t22211x.reg[2] >> 2;
		control_bank(prgRom32kMax)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
	}

	{
		DBWORD bank;

		if (type == T22211B) {
			value = (((save ^ t22211x.reg[2]) >> 3) & 0x02)
			        		| (((save ^ t22211x.reg[2]) >> 5) & 0x01);
		} else {
			value = t22211x.reg[2];
		}

		control_bank(chrRom8kMax)
		bank = value << 13;

		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.data[bank | 0x1C00];

	}
}
BYTE extcl_cpu_rd_mem_Txc_t22211x(WORD address, BYTE openbus, BYTE before) {
	if (address != 0x4100) {
		return (openbus);
	}

	if (type == T22211C) {
		return ((t22211x.reg[1] ^ t22211x.reg[2]) | 0x41);
	} else {
		return ((t22211x.reg[1] ^ t22211x.reg[2]) | 0x40);
	}
}
BYTE extcl_save_mapper_Txc_t22211x(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, t22211x.reg);

	return (EXIT_OK);
}
