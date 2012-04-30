/*
 * mapper242.c
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax;

void mapInit_242(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;

	EXTCLCPUWRMEM(242);

	mapPrgRom8k(4, 0, 0);
}
void extclCpuWrMem_242(WORD address, BYTE value) {
	if (address & 0x0002) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	value = (address & 0x0078) >> 3;
	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}
