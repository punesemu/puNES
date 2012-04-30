/*
 * mapper230.c
 *
 *  Created on: 05/feb/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom16kMax;

void mapInit_230(void) {
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCLCPUWRMEM(230);
	EXTCLSAVEMAPPER(230);
	mapper.intStruct[0] = (BYTE *) &m230;
	mapper.intStructSize[0] = sizeof(m230);

	if (info.reset >= HARD) {
		m230.mode = 0;
	} else {
		m230.mode ^= 1;
	}

	if (m230.mode) {
		mapPrgRom8k(2, 0, 0);
		mapPrgRom8k(2, 2, 7);

		mirroring_V();
	} else {
		mapPrgRom8k(2, 0, 8);
		mapPrgRom8k(2, 2, prgRom16kMax);
	}
}
void extclCpuWrMem_230(WORD address, BYTE value) {
	BYTE save = value;

	if (!m230.mode) {
		value = (save & 0x1F) + 0x08;
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);

		value |= ((~save >> 5) & 0x01);
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 2, value);

		if (save & 0x40) {
			mirroring_V();
		} else {
			mirroring_H();
		}
	} else {
		controlBankWithAND(0x07, prgRom16kMax)
		mapPrgRom8k(2, 0, value);
	}
	mapPrgRom8kUpdate();
}
BYTE extclSaveMapper_230(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m230.mode);

	return (EXIT_OK);
}
