/*
 * mapper51.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom16kMax, prgRom8kMax;
BYTE *prg6000;

void mapInit_51(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;

	EXTCLCPUWRMEM(51);
	EXTCLCPURDMEM(51);
	EXTCLSAVEMAPPER(51);
	mapper.intStruct[0] = (BYTE *) &m51;
	mapper.intStructSize[0] = sizeof(m51);

	if (info.reset >= HARD) {
		memset(&m51, 0x00, sizeof(m51));

		extclCpuWrMem_51(0x6000, 0x02);
	}

	info.mapperExtendWrite = TRUE;
}
void extclCpuWrMem_51(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	if (address >= 0xE000) {
		m51.bank = value & 0x0F;
	} else if (address >= 0xC000) {
		m51.bank = value & 0x0F;
		m51.mode = ((value >> 3) & 0x02) | (m51.mode & 0x01);
	} else if (address >= 0x8000) {
		m51.bank = value & 0x0F;
	} else {
		m51.mode = ((value >> 3) & 0x02) | ((value >> 1) & 0x01);
	}

	if (m51.mode & 0x01) {
		m51.prg6000 = 0x23;

		value = m51.bank;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
	} else {
		m51.prg6000 = 0x2F;

		value = (m51.bank << 1) | (m51.mode >> 1);
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 0, value);

		value = (m51.bank << 1) | 0x07;
		controlBank(prgRom16kMax)
		mapPrgRom8k(2, 2, value);
	}
	mapPrgRom8kUpdate();

	m51.prg6000 = m51.prg6000 | (m51.bank << 2);
	_controlBank(m51.prg6000, prgRom8kMax)
	prg6000 = &prg.rom[m51.prg6000 << 13];

	if (m51.mode == 0x03) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
BYTE extclCpuRdMem_51(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg6000[address & 0x1FFF]);
}
BYTE extclSaveMapper_51(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m51.bank);
	savestateEle(mode, slot, m51.mode);
	savestateEle(mode, slot, m51.prg6000);

	if (mode == SSREAD) {
		prg6000 = &prg.rom[m51.prg6000 << 13];
	}

	return (EXIT_OK);
}
