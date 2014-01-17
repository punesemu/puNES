/*
 * mapper_CPROM.c
 *
 *  Created on: 14/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD chr_rom_4k_max;

void map_init_CPROM(void) {
	info.chr.rom.banks_8k = 2;
	info.chr.rom.banks_4k = 4;
	info.chr.rom.banks_1k = 16;

	chr_rom_4k_max = info.chr.rom.banks_4k - 1;

	if (info.reset >= HARD) {
		chr.bank_1k[4] = &chr.data[0x0000];
		chr.bank_1k[5] = &chr.data[0x0400];
		chr.bank_1k[6] = &chr.data[0x0800];
		chr.bank_1k[7] = &chr.data[0x0C00];
	}

	EXTCL_CPU_WR_MEM(CPROM);
}
void extcl_cpu_wr_mem_CPROM(WORD address, BYTE value) {
	DBWORD bank;

	/* bus conflict */
	value &= prg_rom_rd(address);

	control_bank_with_AND(0x03, chr_rom_4k_max)
	bank = value << 12;
	chr.bank_1k[4] = &chr.data[bank];
	chr.bank_1k[5] = &chr.data[bank | 0x0400];
	chr.bank_1k[6] = &chr.data[bank | 0x0800];
	chr.bank_1k[7] = &chr.data[bank | 0x0C00];
}
