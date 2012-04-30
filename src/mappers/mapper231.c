/*
 * mapper231.c
 *
 *  Created on: 06/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax;

void mapInit_231(void) {
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCLCPUWRMEM(231);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extclCpuWrMem_231(WORD address, BYTE value) {
	value = address & 0x1E;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);

	value |= ((address >> 5) & 0x01);
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 2, value);

	mapPrgRom8kUpdate();

	switch ((address & 0xC0) >> 6) {
		case 0:
			mirroring_SCR0();
			break;
		case 1:
			mirroring_V();
			break;
		case 2:
			mirroring_H();
			break;
		case 3:
			mirroring_SCR0x1_SCR1x3();
			break;
	}
}
