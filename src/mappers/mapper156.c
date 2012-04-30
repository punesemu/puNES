/*
 * mapper156.c
 *
 *  Created on: 4/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom16kMax, chrRom1kMax;

void mapInit_156(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(156);

	mirroring_SCR0();
}
void extclCpuWrMem_156(WORD address, BYTE value) {
	switch (address) {
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			controlBank(chrRom1kMax)
			chr.bank1k[address & 0x0007] = &chr.data[value << 10];
			return;
		case 0xC008:
		case 0xC009:
		case 0xC00A:
		case 0xC00B:
			controlBank(chrRom1kMax)
			chr.bank1k[(address & 0x000F) - 4] = &chr.data[value << 10];
			return;
		case 0xC010:
			controlBank(prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
	}
}
