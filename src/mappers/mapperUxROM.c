/*
 * mapperUxROM.c
 *
 *  Created on: 18/mag/2010
 *      Author: fhorse
 */

#include "memmap.h"
#include "mappers.h"

WORD prgRom16kMax;

void mapInit_UxROM(BYTE model) {
	prgRom16kMax = info.prgRom16kCount - 1;

	switch (model) {
		case UNLROM:
			EXTCL_CPU_WR_MEM(UnlROM);
			break;
		case UXROM:
			EXTCL_CPU_WR_MEM(UxROM);
			break;
		case UNL1XROM:
			EXTCL_CPU_WR_MEM(Unl1xROM);
			break;
		case UNROM180:
			EXTCL_CPU_WR_MEM(UNROM_180);
			break;
	}
}

void extcl_cpu_wr_mem_UxROM(WORD address, BYTE value) {
	/* bus conflict */
	value &= prgRomRd(address);

	controlBankWithAND(0x0F, prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();
}

void extcl_cpu_wr_mem_Unl1xROM(WORD address, BYTE value) {
	/* bus conflict */
	value = (value & prgRomRd(address)) >> 2;

	controlBankWithAND(0x0F, prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();
}

void extcl_cpu_wr_mem_UNROM_180(WORD address, BYTE value) {
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 2, value);
	mapPrgRom8kUpdate();
}

void extcl_cpu_wr_mem_UnlROM(WORD address, BYTE value) {
	controlBankWithAND(0x0F, prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();
}
