/*
 * mapper176.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_176(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(176);

	info.mapperExtendWrite = TRUE;
}
void extcl_cpu_wr_mem_176(WORD address, BYTE value) {
	switch (address) {
		case 0x5FF1:
			value >>= 1;
			controlBank(prgRom32kMax)
			mapPrgRom8k(4, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x5FF2: {
			DBWORD bank;

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
			return;
		}
	}
}
