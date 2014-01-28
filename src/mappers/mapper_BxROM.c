/*
 * mapper_BxROM.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD chr_rom_4k_max;

void map_init_BxROM(void) {
	chr_rom_4k_max = info.chr.rom.banks_4k - 1;

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	switch (info.mapper.from_db) {
		case BXROMUNL:
			EXTCL_CPU_WR_MEM(BxROM_UNL);
			break;
		case AVENINA001:
			info.mapper.extend_wr = TRUE;
			EXTCL_CPU_WR_MEM(AveNina001);
			break;
		default:
			EXTCL_CPU_WR_MEM(BxROM);
			break;
	}
}

void extcl_cpu_wr_mem_BxROM(WORD address, BYTE value) {
	/* bus conflict */
	value &= prg_rom_rd(address);

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_BxROM_UNL(WORD address, BYTE value) {
	control_bank_with_AND(0x3F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_AveNina001(WORD address, BYTE value) {
	DBWORD bank;

	if (address >= 0x8000) {
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
	}

	switch (address) {
		case 0x7FFD:
			control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			break;
		case 0x7FFE:
			control_bank_with_AND(0x1F, chr_rom_4k_max)
			bank = value << 12;
			chr.bank_1k[0] = &chr.data[bank];
			chr.bank_1k[1] = &chr.data[bank | 0x0400];
			chr.bank_1k[2] = &chr.data[bank | 0x0800];
			chr.bank_1k[3] = &chr.data[bank | 0x0C00];
			break;
		case 0x7FFF:
			control_bank_with_AND(0x1F, chr_rom_4k_max)
			bank = value << 12;
			chr.bank_1k[4] = &chr.data[bank];
			chr.bank_1k[5] = &chr.data[bank | 0x0400];
			chr.bank_1k[6] = &chr.data[bank | 0x0800];
			chr.bank_1k[7] = &chr.data[bank | 0x0C00];
			break;
	}
}
