/*
 * mapper240.c
 *
 *  Created on: 24/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_240(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = (info.chrRom4kCount >> 1) - 1;

	EXTCL_CPU_WR_MEM(240);

	info.mapperExtendWrite = TRUE;
}
void extcl_cpu_wr_mem_240(WORD address, BYTE value) {
	if ((address >= 0x4020) && (address < 0x6000)) {
		DBWORD bank;
		BYTE save = value;

		value >>= 4;
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
}
