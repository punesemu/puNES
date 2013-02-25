/*
 * mapperRcm.c
 *
 *  Created on: 23/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_Rcm(BYTE type) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = (info.chrRom4kCount >> 1) - 1;

	switch (type) {
		case GS2015:
			EXTCL_CPU_WR_MEM(GS2015);

			if (info.reset >= HARD) {
				mapPrgRom8k(4, 0, 0);
			}
			break;
	}
}
void extcl_cpu_wr_mem_GS2015(WORD address, BYTE value) {
	DBWORD bank;

	value = address;
	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();

	value = address >> 1;
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
