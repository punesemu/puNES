/*
 * mapperCaltron.c
 *
 *  Created on: 16/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_Caltron(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(Caltron);

	info.mapperExtendWrite = TRUE;

	if (info.reset >= HARD) {
		caltron.reg = 0;
		mapPrgRom8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_Caltron(WORD address, BYTE value) {
	DBWORD bank;

	if (address < 0x6000) {
		return;
	}

	if ((address >= 0x6000) && (address < 0x6800)) {
		caltron.reg = value = address & 0x00FF;

		controlBankWithAND(0x07, prgRom32kMax)
		mapPrgRom8k(4, 0, value);
		mapPrgRom8kUpdate();

		if (caltron.reg & 0x10) {
			mirroring_H();
		} else {
			mirroring_V();
		}
		return;
	}

	if (address < 0x8000) {
		return;
	}

	if (caltron.reg & 0x04) {
		value = ((caltron.reg >> 1) & 0x0C) | (value & 0x03);
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
BYTE extcl_save_mapper_Caltron(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, caltron.reg);

	return (EXIT_OK);
}
