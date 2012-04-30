/*
 * mapperHes.c
 *
 *  Created on: 26/set/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_Hes(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCLCPUWRMEM(Hes);

	info.mapperExtendWrite = TRUE;

	if (info.reset >= HARD) {
		if (prgRom32kMax != 0xFFFF) {
			mapPrgRom8k(4, 0, 0);
		}
	}
}
void extclCpuWrMem_Hes(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if ((address & 0x0100) == 0x0100) {
		const BYTE save = value;
		DBWORD bank;

		if (value & 0x80) {
			mirroring_V();
		} else {
			mirroring_H();
		}

		if (prgRom32kMax != 0xFFFF) {
			value = (value >> 3) & 0x07;
			controlBank(prgRom32kMax)
			mapPrgRom8k(4, 0, value);
			mapPrgRom8kUpdate();
		}

		value = ((save >> 3) & 0x08) | (save & 0x07);
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
}
