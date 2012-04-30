/*
 * mapper246.c
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom8kMax, chrRom2kMax;

void mapInit_246(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;

	EXTCLCPUWRMEM(246);
	EXTCLCPURDMEM(246);

	if (info.reset >= HARD) {
		mapPrgRom8kReset();
	}

	info.mapperExtendWrite = TRUE;
}
void extclCpuWrMem_246(WORD address, BYTE value) {
	BYTE reg, slot;
	DBWORD bank;

	if ((address < 0x6000) || (address > 0x67FF)) {
		return;
	}

	reg = address & 0x07;

	if (reg < 4) {
		controlBank(prgRom8kMax)
		mapPrgRom8k(1, reg, value);
		mapPrgRom8kUpdate();
		return;
	}

	slot = (reg - 4) << 1;
	controlBank(chrRom2kMax)
	bank = value << 11;
	chr.bank1k[slot] = &chr.data[bank];
	chr.bank1k[slot + 1] = &chr.data[bank | 0x0400];
}
BYTE extclCpuRdMem_246(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x67FF)) {
		return (openbus);
	}

	return(before);
}
