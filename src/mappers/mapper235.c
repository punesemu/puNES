/*
 * mapper235.c
 *
 *  Created on: 11/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

static const BYTE slots[4][4][2] = {
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x00, 1 }, { 0x00, 1 } },
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x20, 0 }, { 0x00, 1 } },
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x20, 0 }, { 0x40, 0 } },
	{ { 0x00, 0 }, { 0x20, 0 }, { 0x40, 0 }, { 0x60, 0 } }
};

WORD prgRom32kMax, prgRom16kMax;
BYTE type;

void mapInit_235(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;

	switch (info.prgRom16kCount) {
		case 64:
			type = 0;
			break;
		case 128:
			type = 1;
			break;
		case 192:
			type = 2;
			break;
		case 256:
		default:
			type = 3;
			break;
	}

	EXTCLCPUWRMEM(235);
	if (type != 3) {
		EXTCLCPURDMEM(235);
		EXTCLSAVEMAPPER(235);
		mapper.intStruct[0] = (BYTE *) &m235;
		mapper.intStructSize[0] = sizeof(m235);

		info.mapperExtendRead = TRUE;
	}

	if (info.reset >= HARD) {
        m235.openbus = 0;
        extclCpuWrMem_235(0x8000, 0x00);
	}
}
void extclCpuWrMem_235(WORD address, BYTE value) {
	BYTE bank = slots[type][(address >> 8) & 0x03][0] | (address & 0x1F);
	m235.openbus = slots[type][(address >> 8) & 0x03][1];

	if (address & 0x0800) {
		value = (bank << 1) | ((address >> 12) & 0x01);
    	controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
    	mapPrgRom8k(2, 2, value);
	} else {
		value = bank;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
	}
	mapPrgRom8kUpdate();

	if (address & 0x0400) {
		mirroring_SCR0();
	} else if (address & 0x2000) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
BYTE extclCpuRdMem_235(WORD address, BYTE openbus, BYTE before) {
	if (!m235.openbus || (address < 0x8000)) {
		return (openbus);
	}

	return (address >> 8);
}
BYTE extclSaveMapper_235(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m235.openbus);

	return (EXIT_OK);
}
