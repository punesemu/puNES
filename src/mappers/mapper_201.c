/*
 * mapper_201.c
 *
 *  Created on: 21/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_32k_max, chr_rom_8k_max;

void map_init_201(void) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;
	chr_rom_8k_max = (info.chr.rom.banks_4k >> 1) - 1;

	EXTCL_CPU_WR_MEM(201);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_201(0x8000, 0);
	}
}
void extcl_cpu_wr_mem_201(WORD address, BYTE value) {
	DBWORD bank;

	value = (address & 0xFF);
	control_bank(prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = (address & 0xFF);
	control_bank(chr_rom_8k_max)
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
