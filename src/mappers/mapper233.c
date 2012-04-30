/*
 * mapper233.c
 *
 *  Created on: 10/feb/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, prgRom16kMax;

void mapInit_233(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCLCPUWRMEM(233);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extclCpuWrMem_233(WORD address, BYTE value) {
	BYTE save = value;

	value &= 0x1F;

    if (save & 0x20) {
    	controlBank(prgRom16kMax)
    	mapPrgRom8k(2, 0, value);
    	mapPrgRom8k(2, 2, value);
    } else {
    	value >>= 1;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
    }
	mapPrgRom8kUpdate();

	switch ((save & 0xC0) >> 6) {
		case 0:
			mirroring_SCR0x3_SCR1x1();
			break;
		case 1:
			mirroring_V();
			break;
		case 2:
			mirroring_H();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}
