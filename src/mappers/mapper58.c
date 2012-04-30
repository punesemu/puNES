/*
 * mapper58.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax, chrRom8kMax;

void mapInit_58(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCLCPUWRMEM(58);

	if (info.reset >= HARD) {
		extclCpuWrMem_58(0x8000, 0x00);
	}
}
void extclCpuWrMem_58(WORD address, BYTE value) {
	DBWORD bank;
	BYTE tmp = address & 0x07;

	if (address & 0x0080) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	value = tmp & ~((~address >> 6) & 0x01);
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);

	value = tmp | ((~address >> 6) & 0x01);
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 2, value);

	mapPrgRom8kUpdate();

	value = (address >> 3) & 0x07;
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
