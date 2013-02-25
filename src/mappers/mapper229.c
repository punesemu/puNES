/*
 * mapper229.c
 *
 *  Created on: 05/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax, chrRom8kMax;

void mapInit_229(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(229);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_229(WORD address, BYTE value) {
	DBWORD bank;

	value = address & 0x1F;

	if (address & 0x001E) {
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);
	} else {
		mapPrgRom8k(4, 0, 0);
	}
	mapPrgRom8kUpdate();

	if (address & 0x0020) {
		mirroring_H();
	} else {
		mirroring_V();
	}

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
