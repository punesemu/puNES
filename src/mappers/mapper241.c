/*
 * mapper241.c
 *
 *  Created on: 24/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"

WORD prgRom32kMax;

void mapInit_241(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;

	EXTCL_CPU_WR_MEM(241);
	EXTCL_CPU_RD_MEM(241);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_241(WORD address, BYTE value) {
	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}
BYTE extcl_cpu_rd_mem_241(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x4020) && (address < 0x6000)) {
		return (0x50);
	}

	return (openbus);
}
