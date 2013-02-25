/*
 * mapperMagic.c
 *
 *  Created on: 22/set/2010
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_Magic(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(Magic);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_Magic(WORD address, BYTE value) {
	const BYTE save = value;
	DBWORD bank;

	value >>= 1;
	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
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
