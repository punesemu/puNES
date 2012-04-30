/*
 * mapperWhirlwind.c
 *
 *  Created on: 25/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom8kMax;

void mapInit_Whirlwind(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom8kMax = info.prgRom8kCount - 1;

	EXTCLCPUWRMEM(Whirlwind);
	EXTCLCPURDMEM(Whirlwind);
	EXTCLSAVEMAPPER(Whirlwind);
	mapper.intStruct[0] = (BYTE *) &whirlwind;
	mapper.intStructSize[0] = sizeof(whirlwind);

	info.prgRamPlus8kCount = FALSE;

	if (info.reset >= HARD) {
		memset(&whirlwind, 0x00, sizeof(whirlwind));

		mapPrgRom8k(4, 0, prgRom32kMax);
	}
}
void extclCpuWrMem_Whirlwind(WORD address, BYTE value) {
	if (address == 0x8FFF) {
		controlBank(prgRom8kMax)
		whirlwind.prgRam = value << 13;
	}
}
BYTE extclCpuRdMem_Whirlwind(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg.rom[whirlwind.prgRam + (address - 0x6000)]);
}
BYTE extclSaveMapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, whirlwind.prgRam);

	return (EXIT_OK);
}
