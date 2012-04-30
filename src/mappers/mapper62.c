/*
 * mapper62.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, prgRom16kMax, chrRom8kMax;

void mapInit_62(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCLCPUWRMEM(62);

	if (info.reset >= HARD) {
		extclCpuWrMem_62(0x8000, 0x00);
	}
}
void extclCpuWrMem_62(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0080) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	/*
	 * workaround per far funzionare correttamente "Fancy Mario"
	 * della rom "Super 700-in-1 [p1][!].nes" che non utilizza ne il mirroring
	 * verticale ne quello orizzontale.
	 */
	if ((info.mapperType == SUPER700IN1) && (address == 0x8790)) {
		mirroring_FSCR();
	}

	value = ((address & 0x1F) << 2) | (value & 0x03);
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

	if (address & 0x0020) {
		value = (address & 0x40) | ((address >> 8) & 0x3F);
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);
	} else {
		value = ((address & 0x40) | ((address >> 8) & 0x3F)) >> 1;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
	}
	mapPrgRom8kUpdate();
}
