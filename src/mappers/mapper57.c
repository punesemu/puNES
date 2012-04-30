/*
 * mapper57.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom16kMax, chrRom8kMax;

void mapInit_57(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCLCPUWRMEM(57);
	EXTCLSAVEMAPPER(57);
	mapper.intStruct[0] = (BYTE *) &m57;
	mapper.intStructSize[0] = sizeof(m57);

	if (info.reset >= HARD) {
		memset(&m57, 0x00, sizeof(m57));

		extclCpuWrMem_57(0x8800, 0x00);
	}
}
void extclCpuWrMem_57(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0800) {
		m57.reg[0] = value;

		if (m57.reg[0] & 0x08) {
			mirroring_H();
		} else  {
			mirroring_V();
		}

		if (m57.reg[0] & 0x10) {
			value = (m57.reg[0] & 0xC0) >> 6;
			controlBank(prgRom32kMax)
			mapPrgRom8k(4, 0, value);
		} else {
			value = (m57.reg[0] & 0xE0) >> 5;
			controlBank(prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8k(2, 2, value);
		}

		mapPrgRom8kUpdate();
	} else {
		m57.reg[1] = value;
	}

	value = (m57.reg[1] & 0x07) | (m57.reg[0] & 0x07) | ((m57.reg[1] & 0x40) >> 3);
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
BYTE extclSaveMapper_57(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m57.reg);

	return (EXIT_OK);
}
