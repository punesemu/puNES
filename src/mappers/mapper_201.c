/*
 * mapper_201.c
 *
 *  Created on: 21/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_201(void) {
	EXTCL_CPU_WR_MEM(201);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_201(0x8000, 0);
	}
}
void extcl_cpu_wr_mem_201(WORD address, BYTE value) {
	DBWORD bank;

	value = (address & 0xFF);
	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = (address & 0xFF);
	control_bank(info.chr.rom.max.banks_8k)
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
