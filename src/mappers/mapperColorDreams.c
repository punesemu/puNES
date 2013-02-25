/*
 * mapperColorDreams.c
 *
 *  Created on: 11/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_ColorDreams(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(ColorDreams);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_ColorDreams(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD chrBank;

	/* bus conflict */
	if (info.mapperType != CDNOCONFLCT) {
		save = value &= prgRomRd(address);
	}

	controlBankWithAND(0x03, prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();

	value = (save & 0xF0) >> 4;
	controlBank(chrRom8kMax)
	chrBank = value << 13;
	chr.bank1k[0] = &chr.data[chrBank];
	chr.bank1k[1] = &chr.data[chrBank | 0x0400];
	chr.bank1k[2] = &chr.data[chrBank | 0x0800];
	chr.bank1k[3] = &chr.data[chrBank | 0x0C00];
	chr.bank1k[4] = &chr.data[chrBank | 0x1000];
	chr.bank1k[5] = &chr.data[chrBank | 0x1400];
	chr.bank1k[6] = &chr.data[chrBank | 0x1800];
	chr.bank1k[7] = &chr.data[chrBank | 0x1C00];
}
