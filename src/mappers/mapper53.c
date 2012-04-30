/*
 * mapper53.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom16kMax, prgRom8kMax;
BYTE *prg6000;

void mapInit_53(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;

	EXTCLCPUWRMEM(53);
	EXTCLCPURDMEM(53);
	EXTCLSAVEMAPPER(53);
	mapper.intStruct[0] = (BYTE *) &m53;
	mapper.intStructSize[0] = sizeof(m53);

	if (info.reset >= HARD) {
		memset(&m53, 0x00, sizeof(m53));

		extclCpuWrMem_53(0x6000, 0x00);
	}

	info.mapperExtendWrite = TRUE;
}
void extclCpuWrMem_53(WORD address, BYTE value) {
	BYTE tmp;

	if (address < 0x6000) {
		return;
	}

	if (address >= 0x8000) {
		m53.reg[1] = value;
	} else {
		m53.reg[0] = value;

		if (m53.reg[0] & 0x20) {
			mirroring_H();
		} else  {
			mirroring_V();
		}
	}

	tmp = (m53.reg[0] << 3) & 0x78;

	m53.prg6000 = ((tmp << 1) | 0x0F) + 4;
	_controlBank(m53.prg6000, prgRom8kMax)
	prg6000 = &prg.rom[m53.prg6000 << 13];

	value = (m53.reg[0] & 0x10) ? (tmp | (m53.reg[1] & 0x07)) + 2 : 0;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);

	value = (m53.reg[0] & 0x10) ? (tmp | (0xFF & 0x07)) + 2 : 1;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 2, value);

	mapPrgRom8kUpdate();
}
BYTE extclCpuRdMem_53(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg6000[address & 0x1FFF]);
}
BYTE extclSaveMapper_53(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m53.reg);
	savestateEle(mode, slot, m53.prg6000);

	if (mode == SSREAD) {
		prg6000 = &prg.rom[m53.prg6000 << 13];
	}

	return (EXIT_OK);
}
