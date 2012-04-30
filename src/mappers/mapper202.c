/*
 * mapper202.c
 *
 *  Created on: 21/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax, chrRom8kMax;

void mapInit_202(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = (info.chrRom4kCount >> 1) - 1;

	EXTCLCPUWRMEM(202);

	info.mapperExtendWrite = TRUE;

	if (info.reset >= HARD) {
		extclCpuWrMem_202(0x8000, 0);
	}
}
void extclCpuWrMem_202(WORD address, BYTE value) {
	BYTE save = (address >> 1) & 0x07;
	DBWORD bank;

	if (address < 0x4020) {
		return;
	}

	value = save;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	if ((address & 0x0C) == 0x0C) {
		value = save + 1;
		controlBank(prgRom16kMax)
	}
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

	if (address & 0x0001) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
