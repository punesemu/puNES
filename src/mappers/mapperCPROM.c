/*
 * mapperCPROM.c
 *
 *  Created on: 14/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD chrRom4kMax;

void mapInit_CPROM(void) {
	info.chrRom8kCount = 2;
	info.chrRom4kCount = 4;
	info.chrRom1kCount = 16;

	chrRom4kMax = info.chrRom4kCount - 1;

	if (info.reset >= HARD) {
		chr.bank1k[4] = &chr.data[0x0000];
		chr.bank1k[5] = &chr.data[0x0400];
		chr.bank1k[6] = &chr.data[0x0800];
		chr.bank1k[7] = &chr.data[0x0C00];
	}

	EXTCL_CPU_WR_MEM(CPROM);
}
void extcl_cpu_wr_mem_CPROM(WORD address, BYTE value) {
	DBWORD bank;

	/* bus conflict */
	value &= prgRomRd(address);

	controlBankWithAND(0x03, chrRom4kMax)
	bank = value << 12;
	chr.bank1k[4] = &chr.data[bank];
	chr.bank1k[5] = &chr.data[bank | 0x0400];
	chr.bank1k[6] = &chr.data[bank | 0x0800];
	chr.bank1k[7] = &chr.data[bank | 0x0C00];
}
