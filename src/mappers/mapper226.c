/*
 * mapper226.c
 *
 *  Created on: 08/feb/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom16kMax;

void mapInit_226(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCLCPUWRMEM(226);
	EXTCLSAVEMAPPER(226);
	mapper.intStruct[0] = (BYTE *) &m226;
	mapper.intStructSize[0] = sizeof(m226);

	if (info.reset >= HARD) {
		mapPrgRom8k(4, 0, 0);
		memset(&m226, 0x00, sizeof(m226));
	}
}
void extclCpuWrMem_226(WORD address, BYTE value) {
	BYTE bank;

	m226.reg[address & 0x0001] = value;

	bank = ((m226.reg[0] >> 1) & 0x0F) | ((m226.reg[0] >> 3) & 0x10) |
			((m226.reg[1] << 5) & 0x20);

	if (m226.reg[0] & 0x20) {
		value = (bank << 1) | (m226.reg[0] & 0x01);
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);
		mapPrgRom8k(2, 2, value);
	} else {
		value = bank;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
    }
	mapPrgRom8kUpdate();

	if (m226.reg[0] & 0x40) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
BYTE extclSaveMapper_226(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m226.reg);

	return (EXIT_OK);
}
