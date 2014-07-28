/*
 * mapper_BxROM.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

void map_init_BxROM(void) {
	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	switch (info.mapper.submapper) {
		case BXROMBC:
			EXTCL_CPU_WR_MEM(BxROM);
			break;
		case AVENINA001:
			info.mapper.extend_wr = TRUE;
			EXTCL_CPU_WR_MEM(AveNina001);
			break;
		default:
		case BXROMUNL:
			EXTCL_CPU_WR_MEM(BxROM_UNL);
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
			control_bank_with_AND(0x1F, info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			break;
		case 0x7FFF:
			control_bank_with_AND(0x1F, info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
			break;
	}
}
