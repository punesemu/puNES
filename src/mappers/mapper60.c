/*
 * mapper60.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom16kMax, chrRom8kMax;

void mapInit_60(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(60);
	EXTCL_SAVE_MAPPER(60);
	mapper.intStruct[0] = (BYTE *) &m60;
	mapper.intStructSize[0] = sizeof(m60);

	if (info.reset >= HARD) {
		m60.index = 0;
	} else {
		BYTE tmp = m60.index;
		m60.index = ++tmp & 0x03;
	}

	{
		BYTE value;
		DBWORD bank;

		value = m60.index;
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);

		value = m60.index;
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
void extcl_cpu_wr_mem_60(WORD address, BYTE value) {
	return;
}
BYTE extcl_save_mapper_60(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m60.index);

	return (EXIT_OK);
}

void mapInit_60_vt5201(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(60_vt5201);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_60_vt5201(0x8000, 0);
	}
}
void extcl_cpu_wr_mem_60_vt5201(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0008) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	value = (address >> 4) & ~((~address >> 7) & 0x01);
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);

	value = (address >> 4) | ((~address >> 7) & 0x01);
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 2, value);

	mapPrgRom8kUpdate();

	value = address & 0xFF;
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
