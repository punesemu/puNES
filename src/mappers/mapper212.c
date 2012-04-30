/*
 * mapper212.c
 *
 *  Created on: 22/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, prgRom16kMax, chrRom8kMax;

void mapInit_212(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = (info.chrRom4kCount >> 1) - 1;

	EXTCLCPUWRMEM(212);

	if (info.reset >= HARD) {
		extclCpuWrMem_212(0xFFFF, 0);
	}
}
void extclCpuWrMem_212(WORD address, BYTE value) {
	DBWORD bank;

	if (!(address & 0x4000)) {
		/* 0x8000 - 0xB000 */
		value = address;
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);
		mapPrgRom8kUpdate();
	} else {
		/* 0xC000 - 0xF000 */
		value = address >> 1;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
		mapPrgRom8kUpdate();
	}

	value = address;
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

	if (address & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
