/*
 * mapper61.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, prgRom16kMax;

void mapInit_61(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCLCPUWRMEM(61);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extclCpuWrMem_61(WORD address, BYTE value) {
	if (address & 0x0080) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	if (address & 0x0010) {
		value = ((address << 1) & 0x1E) | ((address >> 5) & 0x01);
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);
	} else {
		value = address & 0x0F;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
	}
	mapPrgRom8kUpdate();
}
