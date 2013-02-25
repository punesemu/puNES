/*
 * mapperVs.c
 *
 *  Created on: 22/set/2010
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom8kMax, chrRom8kMax;

void mapInit_Vs(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(Vs);
	EXTCL_CPU_WR_R4016(Vs);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_Vs(WORD address, BYTE value) {
	return;
}
void extcl_cpu_wr_r4016_Vs(BYTE value) {
	const BYTE save = value;
	DBWORD bank;

	value &= 0x04;
	controlBank(prgRom8kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();

	value = save >> 2;
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
