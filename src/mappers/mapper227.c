/*
 * mapper227.c
 *
 *  Created on: 09/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, prgRom16kMax;

void mapInit_227(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCL_CPU_WR_MEM(227);

	extcl_cpu_wr_mem_227(0x8000, 0x00);
}
void extcl_cpu_wr_mem_227(WORD address, BYTE value) {
	BYTE bank = ((address >> 4) & 0x10) | ((address >> 3) & 0x0F);

    if (address & 0x0001) {
    	value = bank;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
    } else {
    	value = (bank << 1) | ((address >> 2) & 0x01);
    	controlBank(prgRom16kMax)
    	mapPrgRom8k(2, 0, value);
    	mapPrgRom8k(2, 2, value);
    }

	if (!(address & 0x0080)) {
    	value = ((address & 0x0200) ? 0x07 : 0x00) | ((bank << 1) & 0x38);
    	controlBank(prgRom16kMax)
    	mapPrgRom8k(2, 2, value);
	}
	mapPrgRom8kUpdate();

	if (address & 0x0002) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
