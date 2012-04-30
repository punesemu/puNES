/*
 * mapperAxROM.c
 *
 *  Created on: 01/mar/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax;

void mapInit_AxROM(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;

	EXTCLCPUWRMEM(AxROM);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}

	if (info.id == BBCARUNL) {
		mirroring_SCR0();
	}
}
void extclCpuWrMem_AxROM(WORD address, BYTE value) {
	/* bus conflict */
	if (info.mapperType == AMROM) {
		value &= prgRomRd(address);
	}

	if (value & 0x10) {
		mirroring_SCR0();
	} else {
		mirroring_SCR1();
	}

	controlBankWithAND(0x0F, prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}
