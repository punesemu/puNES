/*
 * mapper46.c
 *
 *  Created on: 20/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, chrRom8kMax;

void mapInit_46(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;

	EXTCL_CPU_WR_MEM(46);
	EXTCL_SAVE_MAPPER(46);
	mapper.intStruct[0] = (BYTE *) &m46;
	mapper.intStructSize[0] = sizeof(m46);

	if (info.reset >= HARD) {
		memset(&m46, 0x00, sizeof(m46));

		mapPrgRom8k(4, 0, 0);
	}

	info.mapperExtendWrite = TRUE;
}
void extcl_cpu_wr_mem_46(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	if (address >= 0x8000) {
		value = (m46.prg & 0x1E) | (save & 0x01);
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
		mapPrgRom8kUpdate();
		m46.prg = value;

		value = (m46.chr & 0x78) | ((save >> 4) & 0x07);
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
		m46.chr = value;

		return;
	}

	if (address >= 0x6000) {
		value = (m46.prg & 0x01) | ((save << 1) & 0x1E);
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
		mapPrgRom8kUpdate();
		m46.prg = value;

		value = (m46.chr & 0x07) | ((save >> 1) & 0x78);
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
		m46.chr = value;

		return;
	}
}
BYTE extcl_save_mapper_46(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m46.prg);
	savestateEle(mode, slot, m46.chr);
	return (EXIT_OK);
}
