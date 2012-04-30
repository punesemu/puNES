/*
 * mapper203.c
 *
 *  Created on: 21/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax, chrRom8kMax;

void mapInit_203(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = (info.chrRom4kCount >> 1) - 1;

	EXTCLCPUWRMEM(203);

	if (info.reset >= HARD) {
		extclCpuWrMem_203(0x8000, 0);
	}
}
void extclCpuWrMem_203(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	value = save >> 2;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8k(2, 2, value);
	mapPrgRom8kUpdate();

	value = save;
	controlBank(chrRom8kMax)
	bank = value << 13;
	chr.bank1k[0] = &chr.data[bank];
	chr.bank1k[1] = &chr.data[bank | 0x0400];
	chr.bank1k[2] = &chr.data[bank | 0x0800];
	chr.bank1k[3] = &chr.data[bank | 0x0C00];
	chr.bank1k[4] = &chr.data[bank | 0x1000];
	chr.bank1k[5] = &chr.data[bank | 0x1400];
	chr.bank1k[6] = &chr.data[bank | 0x1800];
	chr.bank1k[7] = &chr.data[bank | 0x1C00];
}
