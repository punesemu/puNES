/*
 * mapperHen.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax;
BYTE type;

void mapInit_Hen(BYTE model) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;

	switch (model) {
		case HEN177:
		case HENFANKONG:
			EXTCL_CPU_WR_MEM(Hen_177);
			break;
		case HENXJZB:
			EXTCL_CPU_WR_MEM(Hen_xjzb);
			info.mapperExtendWrite = TRUE;
			break;
	}

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
	}

	type = model;
}

void extcl_cpu_wr_mem_Hen_177(WORD address, BYTE value) {
	if (type != HENFANKONG) {
		if (value & 0x20) {
			mirroring_H();
		} else {
			mirroring_V();
		}
	}

	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}

void extcl_cpu_wr_mem_Hen_xjzb(WORD address, BYTE value) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return;
	}

	value >>= 1;
	controlBank(prgRom32kMax)
	mapPrgRom8k(4, 0, value);
	mapPrgRom8kUpdate();
}
