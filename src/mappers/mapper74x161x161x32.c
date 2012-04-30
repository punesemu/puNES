/*
 * mapper74x161x161x32.c
 *
 *  Created on: 19/set/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax, chrRom8kMax;
BYTE type;

void mapInit_74x161x161x32(BYTE model) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCLCPUWRMEM(74x161x161x32);

	type = model;
}
void extclCpuWrMem_74x161x161x32(WORD address, BYTE value) {
	/* bus conflict */
	const BYTE save = value &= prgRomRd(address);
	DBWORD bank;

	if (type == IC74X161X161X32B) {
		if (value & 0x80) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	}

	controlBankWithAND(0x0F, chrRom8kMax)
	bank = value << 13;
	chr.bank1k[0] = &chr.data[bank];
	chr.bank1k[1] = &chr.data[bank | 0x0400];
	chr.bank1k[2] = &chr.data[bank | 0x0800];
	chr.bank1k[3] = &chr.data[bank | 0x0C00];
	chr.bank1k[4] = &chr.data[bank | 0x1000];
	chr.bank1k[5] = &chr.data[bank | 0x1400];
	chr.bank1k[6] = &chr.data[bank | 0x1800];
	chr.bank1k[7] = &chr.data[bank | 0x1C00];

	value = save >> 4;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();
}
